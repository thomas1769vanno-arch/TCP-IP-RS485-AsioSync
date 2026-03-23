#define ASIO_STANDALONE
#include "asio.hpp"
#include "protocollo.h"
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>

#if !defined(_WIN32) && !defined(_WIN64)
#include <termios.h>
#include <unistd.h>
#endif

using namespace asio;

int main() {
	try {
    	io_context io;
    	// Verifica il nome della porta (es. /dev/ttyUSB0 o /dev/ttyS0)
    	serial_port serial(io, "/dev/ttyUSB0");

    	serial.set_option(serial_port_base::baud_rate(9600));
    	serial.set_option(serial_port_base::character_size(8));
    	serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    	serial.set_option(serial_port_base::parity(serial_port_base::parity::none));
    	serial.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));

#if !defined(_WIN32) && !defined(_WIN64)
    	int fd = serial.native_handle();
    	struct termios t;
    	if (tcgetattr(fd, &t) == 0) {
        	cfmakeraw(&t);
        	t.c_cflag |= (CLOCAL | CREAD);
        	tcsetattr(fd, TCSANOW, &t);
    	}
    	tcflush(fd, TCIOFLUSH);
#endif

    	// Dovrebbe stampare 74 con il nuovo protocollo (1+1+4+64+4)
    	std::cout << "[Slave] Avviato. sizeof(ProtocolPacket)=" << sizeof(ProtocolPacket) << "\n";

    	while (true) {
        	// FASE 1: Cerca sync byte 0x55 ('U')
        	uint8_t sb;
        	asio::error_code ec_sync;
        	read(serial, buffer(&sb, 1), ec_sync);
        	if (ec_sync || sb != 0x55) continue;

        	// FASE 2: Leggi i restanti byte (sizeof(ProtocolPacket) - 1)
        	ProtocolPacket p;
        	std::memset(&p, 0, sizeof(p));
        	p.sync_byte = 0x55;

        	asio::error_code ec;
        	// Leggiamo i restanti 73 byte
        	read(serial, buffer(reinterpret_cast<char*>(&p) + 1, sizeof(p) - 1), ec);
       	 
        	if (ec) {
            	std::cerr << "[Slave] Errore lettura corpo pacchetto: " << ec.message() << "\n";
            	continue;
        	}

        	// DEBUG: Stampa info pacchetto ricevuto
        	// Usiamo std::dec per i numeri e std::hex per i byte raw
        	std::cerr << "[Slave][DEBUG] Ricevuto: device_id=" << (int)p.device_id
                  	<< " size=" << p.data_size
                  	<< " CRC_RX=" << std::hex << p.checksum << std::dec << "\n";

        	// FASE 3: Verifica checksum (CRC-16 Modbus come definito nel nuovo protocollo.h)
        	uint32_t calc = calcola_checksum(p.data, p.data_size);
       	 
        	if (p.checksum != calc) {
            	std::cerr << "[Slave] Checksum errato! ricevuto=" << std::hex << p.checksum
                      	<< " calcolato=" << calc << std::dec << "\n";
            	// Opzionale: stampa i primi byte per capire dove slitta il padding
            	continue;
        	}

        	// FASE 4: Decodifica payload (es. "C1_S2_MOVE_LEFT")
        	// Usiamo p.data_size per limitare la stringa ai soli dati utili
        	std::string payload(p.data, std::min((uint32_t)64, p.data_size));
        	std::cout << "[Slave] Ricevuto comando: " << payload << "\n";

        	// Estrazione req_id (es. "C1_S2")
        	size_t pos1 = payload.find('_');
        	if (pos1 == std::string::npos) continue;
        	size_t pos2 = payload.find('_', pos1 + 1);
        	if (pos2 == std::string::npos) continue;
        	std::string req_id = payload.substr(0, pos2);

        	// FASE 5: Invia ACK "C1_S2_ACK"
        	std::string ack_str = req_id + "_ACK";

        	ProtocolPacket ack_p;
        	std::memset(&ack_p, 0, sizeof(ack_p));
        	ack_p.sync_byte = 0x55;
        	ack_p.device_id = 0; // Lo slave risponde come ID 0 o se stesso
        	ack_p.data_size = static_cast<uint32_t>(ack_str.size());
        	std::memcpy(ack_p.data, ack_str.c_str(), ack_p.data_size);
       	 
        	// Calcolo del checksum sulla stringa ACK
        	ack_p.checksum = calcola_checksum(ack_p.data, ack_p.data_size);

        	// Scrittura sulla seriale dell'intera struct (74 byte)
        	write(serial, buffer(&ack_p, sizeof(ack_p)));
       	 
        	std::cout << "[Slave] ACK inviato: " << ack_str
                  	<< " (CRC: " << std::hex << ack_p.checksum << std::dec << ")\n";
       	 
        	// Un piccolo flush per pulire eventuali residui
        	// tcflush(fd, TCIFLUSH);
    	}

	} catch (std::exception& e) {
    	std::cerr << "[Slave] Eccezione fatale: " << e.what() << "\n";
	}
	return 0;
}
