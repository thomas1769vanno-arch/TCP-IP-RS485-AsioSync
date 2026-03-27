# Protocollo Seriale

## Protocollo attuale — Custom (protocollo.h)

### Struttura pacchetto (`ProtocolPacket`)

```
┌──────────────┬───────────────┬──────────────────┬────────────────┬───────────────┐
│  sync_byte   │   device_id   │    data_size     │     data       │   checksum    │
│   uint8_t    │   uint8_t     │    uint32_t      │   char[64]     │   uint32_t    │
│   1 byte     │   1 byte      │    4 byte        │   64 byte      │   4 byte      │
└──────────────┴───────────────┴──────────────────┴────────────────┴───────────────┘
  Totale: 74 byte (con #pragma pack(push,1))
```

- **sync_byte**: sempre `0x55` ('U')
- **device_id**: ID dispositivo mittente (1 = Gateway, 0 = Slave)
- **data_size**: lunghezza effettiva del payload in `data[]` (max 64)
- **data**: payload stringa (es. `C1_S2_MOVE_LEFT` o `C1_S2_ACK`)
- **checksum**: CRC-16 Modbus calcolato sui `data_size` byte di `data`, memorizzato in uint32_t (16 bit usati, 16 superiori = 0)

### CRC-16 Modbus

Polinomio `0xA001`, seed `0xFFFF`. Vedi `calcola_checksum()` in `protocollo.h`.

### Formato payload

**Comando** (Gateway → Slave): `C<client_id>_S<seq>_COMANDO`
- es. `C1_S2_MOVE_LEFT`

**ACK** (Slave → Gateway): `C<client_id>_S<seq>_ACK`
- es. `C1_S2_ACK`

---

## Prossimo protocollo — LEOMATIC XFER 4.0

Protocollo **proprietario** della telecamera LEOMATIC, comunicazione via seriale RS485.

**Stato**: specifica da ricevere dall'utente.

> Una volta ricevuta la documentazione, documentare qui:
> - Struttura del pacchetto
> - Byte di sincronizzazione
> - Comandi supportati
> - Meccanismo di checksum/CRC
> - Flusso handshake
