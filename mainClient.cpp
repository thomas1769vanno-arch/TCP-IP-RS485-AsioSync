
// Definisce la versione minima di Windows richiesta. Necessario per Asio su Windows.
// Su Linux e altri sistemi POSIX, questa riga non è necessaria.
#if defined(_WIN32) || defined(_WIN64)
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include "asio.hpp"
#include <iostream>
#include <thread>   // Per std::this_thread
#include <chrono>   // Per std::chrono::milliseconds
#include <string>
#include <atomic>   // Per std::atomic (variabili thread-safe)
#include <mutex>    // Per std::mutex (sebbene qui usiamo logiche lock-free tramite atomic e ASIO)

using namespace asio;
using ip::tcp;

/*quando lo lanci, il computer crea due thread distinti all'interno dello stesso processo. Se non ci fosse il secondo thread, il client sarebbe "sordo" durante gli sleep o "muto" mentre aspetta i dati.

Ecco come si dividono il lavoro:

1. Il Thread Principale (Il "Capo")
È quello che entra nel int main().

Cosa fa: Esegue le istruzioni in sequenza. Chiama cam.invia("MOVE_LEFT"), poi incontra lo sleep e si mette a dormire per un secondo, poi manda il secondo comando.

Il suo limite: Se dovesse occuparsi lui di leggere i dati dalla rete, mentre dorme (durante lo sleep) non vedrebbe gli ACK arrivare. Il programma sembrerebbe bloccato.

2. Il Thread di Background (Il "Segretario")
È quello che crei con questa riga: std::thread background_thread([&io]() { io.run(); });.

Cosa fa: Vive "accanto" al thread principale. Il suo unico compito è far girare io.run().

Perché è fondamentale: Mentre il "Capo" dorme, il "Segretario" è sveglissimo. Se arriva un ACK dal Gateway, è questo thread che lo riceve, lo stampa a video e resetta il timer dei 5 minuti.

*/

// Classe che incapsula la logica del Client TCP verso il Gateway
class CameraControl {
    io_context& io_;                        // Riferimento al contesto ASIO (motore asincrono)
    tcp::socket socket_;                    // Socket TCP persistente per inviare/ricevere
    steady_timer inactivity_timer_;         // Timer per calcolare l'inattività (timeout)
    
    // Usiamo std::atomic perché queste variabili vengono lette/scritte 
    // sia dal thread principale (main) sia dal thread di background (io.run)
    std::atomic<int> seq_num_{1};           // Contatore thread-safe per i numeri di sequenza
    std::atomic<bool> running_{true};       // Flag thread-safe per tenere in vita il client
    
    asio::streambuf read_buffer_;           // Buffer dinamico per accodare i dati in arrivo

public:
    CameraControl(io_context& io, const std::string& ip, const std::string& port) 
        : io_(io), socket_(io), inactivity_timer_(io) {
        
        // Risoluzione dell'indirizzo e connessione sincrona iniziale
        tcp::resolver resolver(io_);
        connect(socket_, resolver.resolve(ip, port));
        
        // Avvia per la prima volta il timer di inattività
        reset_timer();
        
        do_read(); // Inizia subito la lettura asincrona degli ACK
    }

    // Resetta (o avvia) il timer dei 5 minuti
    void reset_timer() {
        // Imposta la nuova scadenza del timer a 5 minuti da ora.
        // Se il timer era già in corso, l'operazione precedente viene "annullata"
        inactivity_timer_.expires_after(std::chrono::minutes(5));
        
        // Mette il timer in attesa in modo asincrono (non blocca il programma)
        inactivity_timer_.async_wait([this](const asio::error_code& ec) {
            // Se ec è "falso" (nessun errore), significa che il timer è effettivamente scaduto.
            // Se ec contenesse un errore (es. operation_aborted), significa che il timer è stato resettato prima di scadere.
            if (!ec) {
                std::cout << "\n[Timeout] Inattività per 5 minuti. Chiusura automatica del Client.\n";
                running_ = false;  // Segnala al while() nel main di fermarsi
                socket_.close();   // Chiude il socket, forzando l'uscita delle letture pendenti
            }
        });
    }

    // Invia un comando in modo sincrono al Gateway
    void invia(const std::string& comando) {
        if (!running_) return; // Evita di inviare se siamo già in chiusura
        
        reset_timer(); // Qualsiasi operazione azzera il timer dei 5 minuti
        
        // Preleva il numero attuale e poi lo incrementa (operazione thread-safe)
        int seq = seq_num_++;
        
        // Compone il messaggio: S<seq_num>_COMANDO\n
        std::string msg = "S" + std::to_string(seq) + "_" + comando + "\n";
        
        // Scrittura bloccante sul socket
        write(socket_, buffer(msg));
        std::cout << "[MainClient] Inviato: " << msg;
    }

    // Loop di lettura asincrono: resta sempre in ascolto di risposte dal Gateway
    //questo do_read lo esegue il background thread, non il main thread. Il main thread si occupa solo di inviare i comandi e gestire il timer, mentre do_read è sempre attivo in background per ricevere gli ACK.
    //quindi questo metodo do_read fa parte del io.run()? Esatto, do_read() 
    //è chiamato all'interno del thread di background che esegue io.run(). Quando chiami io.run(), 
    //stai dicendo al tuo programma di iniziare a processare tutti gli eventi asincroni che hai impostato 
    //(come le letture asincrone e i timer). Quindi, quando do_read() viene chiamato per la prima volta, imposta una lettura asincrona sul socket. Se arriva un ACK, 
    //la lambda associata a quella lettura viene eseguita, e alla fine di quella lambda chiami di 
    //nuovo do_read() per continuare ad ascoltare altri ACK. In questo modo, do_read() rimane sempre attivo in background, pronto a ricevere e gestire qualsiasi messaggio in arrivo dal Gateway, senza mai bloccare il thread principale che invia i comandi.
    
    void do_read() {
        // asinc_read_until legge dal socket in background finché non trova un '\n'
        async_read_until(socket_, read_buffer_, '\n',
            [this](const asio::error_code& ec, std::size_t length) {
                //[this](...) { ... } è una funzione anonima (Lambda) che verrà eseguita solo quando la lettura è completata o si verifica un errore:
                if (!ec) { // Se non ci sono stati errori (es. disconnessione)
                    reset_timer(); // Resetta il timer anche se riceviamo qualcosa
                    
                    // Estrae una riga di testo dal buffer di lettura
                    std::istream is(&read_buffer_);
                    std::string line;
                    std::getline(is, line);
                    std::cout << "[MainClient] Ricevuto dal Gateway: " << line << "\n";
                    
                    do_read(); // Si richiama per continuare a restare in ascolto del prossimo ACK
                } else {
                    // Se c'è un errore (il socket è stato chiuso dal server o dal timer),
                    // impostiamo running_ a false per far terminare il programma
                    running_ = false;
                }
            });
    }

    // Metodo getter per permettere al main di controllare lo stato
    bool is_running() const { return running_; }
};

int main() {
    try {
        io_context io;
        CameraControl cam(io, "127.0.0.1", "8080");

        // Avviamo un thread separato (background) che si occuperà ESCLUSIVAMENTE
        // di eseguire il ciclo degli eventi asincroni di ASIO (cioè il do_read() e il timer).
        // Se non facessimo questo thread, chiamare io.run() bloccherebbe l'esecuzione
        // e non potremmo mandare i comandi successivi nel main.
        std::thread background_thread([&io]() { io.run(); }); 

       //ma dove è il codice di io.run? È una funzione che fa parte della classe io_context di ASIO. Quando chiami io.run(), stai dicendo al tuo programma di iniziare a processare tutti gli eventi asincroni che hai impostato (come le letture asincrone e i timer).
       //Il codice di io.run() è implementato all'interno della libreria ASIO
       //una volta che arriva un ack quindi il backgrounf thread si occupa di prenderlo, e poi? Il background thread, una volta che riceve un ACK dal Gateway, esegue la lambda che hai passato a async_read_until. In quella lambda, stampi l'ACK ricevuto e poi chiami di nuovo do_read() per continuare ad ascoltare altri ACK. Quindi il background thread rimane sempre attivo, pronto a ricevere e gestire qualsiasi messaggio in arrivo dal Gateway, senza mai bloccare il thread principale che invia i comandi. 
        // Esecuzione di alcuni comandi di test
        cam.invia("MOVE_LEFT");
        // NOTA SULLA PORTABILITÀ:
        // Sleep() con la 'S' maiuscola è una funzione specifica di Windows.
        // Utilizziamo la versione standard di C++ per la massima portabilità.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        cam.invia("ZOOM_IN");

        // Loop principale del programma:
        // Il main thread resta in un loop leggero finché 'running_' è true.
        // Questo permette al client di rimanere connesso e attendere gli ACK.
        while(cam.is_running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Assicuriamoci di attendere che il thread in background termini
        // (cioè quando il socket viene chiuso e non ci sono più operazioni asincrone in corso)
        if (background_thread.joinable()) background_thread.join(); //spiega: "joinable" è una funzione che verifica se il thread è ancora attivo e può essere unito. 
                //Se il thread è già terminato o non è stato avviato, joinable() restituirà false, evitando così errori di runtime quando si tenta di chiamare join() 
                //su un thread non valido.
    } catch (std::exception& e) { std::cerr << "Errore Main: " << e.what() << "\n"; }
    return 0;
}