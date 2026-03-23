#ifndef PROTOCOLLO_H
#define PROTOCOLLO_H
#include <cstdint>
#include <cstring>

#pragma pack(push, 1)
struct ProtocolPacket {
    uint8_t  sync_byte;  // 'U'
    uint8_t  device_id;
    uint32_t data_size;
    char     data[64];
    uint32_t checksum;   // CRC-16
};
#pragma pack(pop)

inline uint32_t calcola_checksum(const char* dati, uint32_t size) { //checksum a 32 bit? Sì, anche se il nome è "CRC-16", in questo caso stiamo restituendo un uint32_t per semplicità di gestione. Il calcolo del CRC-16 produce un valore a 16 bit, ma lo stiamo memorizzando in un uint32_t per evitare problemi di overflow o segno quando lo confrontiamo o lo stampiamo. In pratica, i primi 16 bit del uint32_t conterranno il valore del CRC-16, e i restanti bit saranno zero. Questo non cambia la logica del calcolo del checksum, che rimane un CRC-16 standard.
    uint16_t crc = 0xFFFF;
    const unsigned char* buffer = (const unsigned char*)dati;
    for (uint32_t pos = 0; pos < size; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; }
            else { crc >>= 1; }
        }
    }
    return static_cast<uint32_t>(crc); //i restanti 16 bit saranno zero
}
#endif 