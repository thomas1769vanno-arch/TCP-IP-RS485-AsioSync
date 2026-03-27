# Active Context

## Stato attuale: ANALYZE

## Cosa stavamo facendo

- Sessione iniziale di setup del progetto con Claude Code
- Letto e compreso l'architettura completa del gateway TCP ↔ RS485
- Inizializzata la Memory Bank da zero
- Discusso del prossimo step: implementazione protocollo **LEOMATIC XFER 4.0**

## Cosa resta da fare

- Ricevere la specifica del protocollo LEOMATIC XFER 4.0 (proprietario, per telecamere via RS485)
- Implementare il nuovo protocollo (nuovo header o aggiornamento `protocollo.h`)
- Adattare master/slave al nuovo formato pacchetto

## Problemi aperti

- La specifica LEOMATIC XFER 4.0 non è ancora stata condivisa — serve documentazione dall'utente

## Note

- I custom slash commands `j-*` sono in `.claude/commands/` ma non appaiono nel menu `/` dell'IDE (comportamento noto)
- La Memory Bank è stata creata in questa sessione
