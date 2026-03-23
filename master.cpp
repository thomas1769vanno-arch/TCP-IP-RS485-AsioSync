
#if defined(_WIN32) || defined(_WIN64)
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include "asio.hpp"
#include "protocollo.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include <memory>
#include <string>
#include <algorithm>

using namespace asio;
using ip::tcp;

std::mutex        serial_tx_mutex;
std::mutex        ack_mutex;
std::condition_variable ack_cv;
std::map<std::string, std::string> pending_acks;

// ── Thread di lettura seriale (ACK dallo Slave) ───────────────────────────────
void serial_read_thread(serial_port& serial) {
    try {
        while (true) {
            // Aspetta il sync byte 0x55, scarta tutto il resto
            uint8_t sb;
            read(serial, buffer(&sb, 1));
            if (sb != 0x55) continue;

            // Leggi i restanti sizeof(ProtocolPacket)-1 byte
            ProtocolPacket p;
            p.sync_byte = 0x55;
            read(serial, buffer(reinterpret_cast<char*>(&p) + 1, sizeof(p) - 1));

            // Verifica integrità
            uint32_t calc = calcola_checksum(p.data, p.data_size);
            if (p.checksum != calc) {
                std::cerr << "[Gateway] Checksum errato da Slave"
                          << " (ricevuto=" << p.checksum
                          << " calcolato=" << calc
                          << " data_size=" << p.data_size << ")\n";
                continue;
            }

            // Formato ACK: "C<cid>_S<seq>_ACK"  es. "C1_S2_ACK"
            std::string ack_msg(p.data, p.data_size);
            std::cout << "[Gateway] ACK da Slave: " << ack_msg << "\n";

            // Estrai req_id = "C1_S2" (fino al secondo '_')
            size_t pos1 = ack_msg.find('_');
            if (pos1 == std::string::npos) continue;
            size_t pos2 = ack_msg.find('_', pos1 + 1);
            if (pos2 == std::string::npos) continue;
            std::string req_id = ack_msg.substr(0, pos2);

            {
                std::lock_guard<std::mutex> lock(ack_mutex);
                pending_acks[req_id] = ack_msg;
            }
            ack_cv.notify_all();
        }
    } catch (std::exception& e) {
        std::cerr << "[Gateway] Errore thread seriale: " << e.what() << "\n";
    }
}

// ── Sessione TCP per un singolo Client ────────────────────────────────────────
void client_session(std::shared_ptr<tcp::socket> socket, int client_id,
                    serial_port& serial) {
    try {
        asio::streambuf buf;
        while (true) {

            // ── Leggi comando dal Client TCP ──────────────────────────────
            asio::read_until(*socket, buf, '\n');
            std::istream is(&buf);
            std::string line;
            std::getline(is, line);

            // Pulizia: rimuovi \r finale (Windows line endings) e spazi
            while (!line.empty() &&
                   (line.back() == '\r' || line.back() == '\n'))
                line.pop_back();

            if (line.empty()) continue;

            // ── Costruisci payload: "C1_S2_MOVE_LEFT" ────────────────────
            std::string payload = "C" + std::to_string(client_id) + "_" + line;

            // Estrai req_id = "C1_S2" (primo e secondo token)
            size_t pos1 = payload.find('_');
            if (pos1 == std::string::npos) {
                std::cerr << "[Gateway] Formato non valido: " << line << "\n";
                continue;
            }
            size_t pos2 = payload.find('_', pos1 + 1);
            if (pos2 == std::string::npos) {
                std::cerr << "[Gateway] Formato non valido: " << line << "\n";
                continue;
            }
            std::string req_id = payload.substr(0, pos2); // "C1_S2"

            // ── FIX RACE CONDITION ────────────────────────────────────────
            // Inserisce il placeholder PRIMA di inviare il pacchetto.
            // Se l'ACK arriva prima che la wait() parta, lo trova già in mappa
            // e non si blocca.
            {
                std::lock_guard<std::mutex> lock(ack_mutex);
                if (pending_acks.find(req_id) == pending_acks.end())
                    pending_acks[req_id] = ""; // placeholder vuoto
            }

            // ── Costruisci e invia pacchetto seriale ──────────────────────
            // IMPORTANTE: data_size deve contenere ESATTAMENTE la lunghezza
            // del payload, senza \0 terminatore e senza byte extra.
            // Il checksum viene calcolato sugli stessi byte che lo Slave leggerà.
            ProtocolPacket p;
            memset(&p, 0, sizeof(p)); // azzera tutto, nessun byte sporco
            p.sync_byte = 0x55;
            p.device_id = 1;
            p.data_size = static_cast<uint32_t>(
                std::min(payload.size(), (size_t)64));
            memcpy(p.data, payload.c_str(), p.data_size);
            p.checksum  = calcola_checksum(p.data, p.data_size);

            std::cout << "[Gateway] Client " << client_id
                      << " -> Slave: " << payload
                      << " (data_size=" << p.data_size
                      << " checksum=" << p.checksum << ")\n";

            {
                std::lock_guard<std::mutex> lock(serial_tx_mutex);
                write(serial, buffer(&p, sizeof(p)));
            }

            // ── Attendi ACK dallo Slave ───────────────────────────────────
            // La wait esce solo quando il valore per req_id è NON vuoto
            // (cioè l'ACK reale è arrivato, non il semplice placeholder)
            std::unique_lock<std::mutex> lock(ack_mutex);
            ack_cv.wait(lock, [&]() {
                auto it = pending_acks.find(req_id);
                return it != pending_acks.end() && !it->second.empty();
            });

            std::string ack_msg = pending_acks[req_id];
            pending_acks.erase(req_id);
            lock.unlock();

            // ── Inoltra ACK al Client TCP: "S2_ACK\n" ────────────────────
            // Rimuoviamo il prefisso "C1_" e aggiungiamo \n
            size_t c_pos = ack_msg.find('_');
            std::string client_ack = ack_msg.substr(c_pos + 1) + "\n";
            write(*socket, buffer(client_ack));

            std::cout << "[Gateway] ACK inviato a Client "
                      << client_id << ": " << client_ack;
        }
    } catch (std::exception& e) {
        std::cout << "[Gateway] Client " << client_id
                  << " disconnesso (" << e.what() << ")\n";
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main() {
    try {
        io_context io;

        // Porta seriale verso lo Slave RS485
        // Cambia "COM3" con la porta che vedi in Gestione Dispositivi
        serial_port serial(io, "COM3");
        serial.set_option(serial_port_base::baud_rate(9600));
        serial.set_option(serial_port_base::character_size(8));
        serial.set_option(serial_port_base::stop_bits(
            serial_port_base::stop_bits::one));
        serial.set_option(serial_port_base::parity(
            serial_port_base::parity::none));
        serial.set_option(serial_port_base::flow_control(
            serial_port_base::flow_control::none));

        // Thread dedicato alla ricezione degli ACK dalla seriale
        std::thread serial_th(serial_read_thread, std::ref(serial));
        serial_th.detach();

        // Server TCP sulla porta 8080
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "[Gateway] In attesa di connessioni TCP sulla porta 8080...\n";

        int client_id_counter = 1;
        while (true) {
            auto socket = std::make_shared<tcp::socket>(io);
            acceptor.accept(*socket);
            std::cout << "[Gateway] Nuovo client connesso. ID: "
                      << client_id_counter << "\n";
            std::thread(client_session, socket, client_id_counter,
                        std::ref(serial)).detach();
            client_id_counter++;
        }

    } catch (std::exception& e) {
        std::cerr << "[Gateway] Errore fatale: " << e.what() << "\n";
    }
    return 0;
}


//funzioona piichè abbiamo fatto delle modifiche: avevo i protocol.h diversi. uno con uint16 e
 //l'altro con uint32, poi ho modificato il master cosi riceve i comandi separatameente invece che 
 //incollarli, il master riceve correttamente ack1 quibndi si sblocca e procede ad inviare il secondo comando 
 //e a ricevere ack2