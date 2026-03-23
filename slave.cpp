
// Definisce la versione minima di Windows richiesta. Necessario per Asio su Windows.
// Su Linux e altri sistemi POSIX, questa riga non è necessaria.
#if defined(_WIN32) || defined(_WIN64)
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include "asio.hpp"
#include "protocollo.h"
#include <iostream>
#include <string>
#include <algorithm>

using namespace asio;

int main() {
    try {
        io_context io;
        // NOTA SULLA PORTABILITÀ:
        // Il nome della porta seriale è specifico del sistema operativo.
        // "COM89" è per Windows. Su Linux, sarebbe qualcosa come "/dev/ttyS1" o "/dev/ttyUSB1".
        // Per un'applicazione reale, questo valore dovrebbe essere letto da un file di configurazione
        // o passato come argomento da riga di comando.
        serial_port serial(io, "COM89"); // Su Linux, usare ad es. "/dev/ttyS1"
        serial.set_option(serial_port_base::baud_rate(9600));

        std::cout << "Slave pronto sulla porta seriale...\n";

        while (true) {
            //cio che fa lo slave è leggere continuamente dalla seriale, 
            //aspettando i comandi del Gateway. Quando ne riceve uno, lo esegue 
            //(simulato qui con una stampa a video) e poi invia un ACK di ritorno al Gateway.
            uint8_t sb;
            read(serial, buffer(&sb, 1));
            if (sb == 0x55) { // 'U' come Sync Byte per differenziare i comandi in ingresso dagli ACK
                ProtocolPacket p;
                p.sync_byte = 0x55;
                read(serial, buffer(reinterpret_cast<char*>(&p) + 1, sizeof(p) - 1));

                if (p.checksum == calcola_checksum(p.data, p.data_size)) {
                    std::string cmd_str(p.data, p.data_size);
                    std::cout << "Comando Eseguito: " << cmd_str << " (CRC OK)\n";

                    // Decodifica della busta. Il formato in ingresso è "C<cid>_S<seq>_COMANDO"
                    // Vogliamo estrarre "C<cid>_S<seq>" per comporre il messaggio di ritorno "C<cid>_S<seq>_ACK"
                    size_t pos1 = cmd_str.find('_');
                    std::string ack_str = "ACK";
                    /*È un valore speciale che significa "non trovato". 
                    Il codice controlla che entrambi gli underscore esistano prima di procedere, evitando crash se il pacchetto è malformato.
                    */
                    if (pos1 != std::string::npos) {
                        size_t pos2 = cmd_str.find('_', pos1 + 1); //cerca il secondo _ partendo da pos1+1
                        if (pos2 != std::string::npos) {
                            std::string req_id = cmd_str.substr(0, pos2); // Estrae es. "C1_S2"
                            ack_str = req_id + "_ACK";                    // Genera "C1_S2_ACK"
                        }
                    }

                    // Prepariamo un pacchetto struct col payload dell'ACK 
                    // per rispedirlo indietro al Gateway su RS485.
                    ProtocolPacket p_ack;
                    p_ack.sync_byte = 0x55; // 'U' come Sync Byte per differenziare gli ACK dai comandi in ingresso
                    p_ack.device_id = 1;
                    p_ack.data_size = std::min(ack_str.length(), (size_t)64);
                    memset(p_ack.data, 0, 64);
                    memcpy(p_ack.data, ack_str.c_str(), p_ack.data_size);
                    p_ack.checksum = calcola_checksum(p_ack.data, p_ack.data_size);

                    // Scrittura fisica immediata  sulla porta seriale.
                    write(serial, buffer(&p_ack, sizeof(p_ack)));
                    std::cout << "Inviato " << ack_str << " al Gateway.\n";
                } else {
                    std::cout << "Errore Integrità Pacchetto!\n";
                }
            }
        }
    } catch (std::exception& e) { std::cerr << "Errore Slave: " << e.what() << "\n"; }
    return 0;
}