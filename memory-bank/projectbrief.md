# Project Brief — TCP-IP-RS485-AsioSync

## Scopo

Gateway C++ che collega client TCP/IP a dispositivi seriali RS485.
Permette a client remoti (es. software su PC) di controllare dispositivi fisici (es. telecamere LEOMATIC) tramite un bridge TCP ↔ RS485.

## Stack tecnologico

- **Linguaggio**: C++17
- **Libreria rete/seriale**: ASIO standalone (header-only, senza Boost)
- **Compilazione**: Windows (MSVC/MinGW) + Linux (GCC)
- **Comunicazione rete**: TCP/IP sincrono (porta 8080)
- **Comunicazione seriale**: RS485 / seriale asincrona (baud 9600, 8N1)

## Architettura

```
[mainClient.cpp]  →  TCP (porta 8080)  →  [master.cpp / Gateway]  →  RS485 seriale  →  [slave.cpp / slaveLinux.cpp]
```

### File principali

| File | Ruolo |
|------|-------|
| `mainClient.cpp` | Client TCP — invia comandi al Gateway, riceve ACK |
| `master.cpp` | Gateway Windows — bridge TCP↔seriale, gestisce multi-client |
| `slave.cpp` | Slave Windows — riceve comandi seriali, esegue, risponde ACK |
| `slaveLinux.cpp` | Slave Linux — stessa logica di slave.cpp, ottimizzato per Linux (`/dev/ttyUSB0`) |
| `protocollo.h` | Definizione struct pacchetto + CRC-16 Modbus |

### Porte seriali configurate

| File | Porta |
|------|-------|
| `master.cpp` | `COM3` (Windows) |
| `slave.cpp` | `COM89` (Windows, emulatore virtuale) |
| `slaveLinux.cpp` | `/dev/ttyUSB0` (Linux) |

## Flusso dati

1. `mainClient` invia: `S<seq>_COMANDO\n` via TCP
2. `master` riceve e costruisce payload: `C<cid>_S<seq>_COMANDO`
3. `master` incapsula in `ProtocolPacket` e invia via RS485
4. `slave` riceve, verifica CRC, esegue comando, risponde `C<cid>_S<seq>_ACK`
5. `master` riceve ACK dalla seriale, lo inoltra al client TCP come `S<seq>_ACK\n`

## Multi-client

Il `master` supporta più client TCP simultanei tramite thread separati (uno per client).
Un thread dedicato legge continuamente gli ACK dalla seriale (`serial_read_thread`).
Race condition risolta con placeholder in `pending_acks` prima dell'invio del pacchetto.
