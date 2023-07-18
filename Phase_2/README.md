# PANDOS PHASE 2

La fase 2 del progetto è focalizzata sulla realizzazione del livello 3 del S.O. Panda+, ciò che riguarda il kernel.
Le funzionalità che deve gestire sono:

- Inizializzazione del sistema

- Scheduling dei processi

- Gestione delle eccezioni:
    -Interrupt
    -Syscall
    -Trap

I file principali della fase 2 sono divisi come segue:

### INITIAL : 

- Inizializza le variabili globali del nucleo e le varie strutture dati

- Crea e inizializza il primo processo

- Invoca lo scheduler

### SCHEDULER :

Il ruolo dello scheduler e’ di decidere quale processo deve entrare in esecuzione

### EXCEPTION :

Gestisce le eccezioni del sistema, identificando il tipo di eccezione avvenuta e richiamando l'handler appropriato per gestire l'eccezione specifica tra cui:

- Interrupt
- TLB Trap
- Program Trap
- Syscall

### INTERRUPT :

Si occupa di determinare l'interrupt facendo distinzione tra:

- Processor Local Timer interrupt

- Interval Timer interrupt

- Device interrupt




## Scelte progettuali e Robe aggiunte
