# Progress — Stato Avanzamento Lavori

## Completato

- [x] Architettura base TCP ↔ RS485 funzionante
- [x] Protocollo custom (`protocollo.h`): `ProtocolPacket` 74 byte, CRC-16 Modbus in uint32_t
- [x] `master.cpp`: Gateway Windows multi-client, thread seriale dedicato per ACK, race condition fix
- [x] `slave.cpp`: Slave Windows
- [x] `slaveLinux.cpp`: Slave Linux con configurazione `termios` per raw mode
- [x] `mainClient.cpp`: Client TCP con timer inattività 5 min, lettura ACK asincrona in background thread

## In corso / Prossimi step

- [ ] **LEOMATIC XFER 4.0**: ricevere specifica del protocollo proprietario
- [ ] Implementare nuovo `protocollo_leomatic.h` (o aggiornare `protocollo.h`) con struttura pacchetti LEOMATIC
- [ ] Adattare `master.cpp` e/o `slave.cpp` al nuovo protocollo

## Note storiche

- Bug risolto: `protocollo.h` aveva discrepanza `uint16_t` vs `uint32_t` tra master e slave → uniformato a `uint32_t`
- Il master riceveva i comandi incollati → fix: ora li gestisce separatamente per ACK
