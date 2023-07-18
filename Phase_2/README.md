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

Variabili aggiunte

- IOValues:
    tra i parametri della system call DoIO l'utente passa l'indirizzo di un array dentro il quale scrive i comandi da dare al device, al completamento dell'operazione lo status del device va scritto in questo array. Il campo IOValues che abbiamo aggiunto al descrittore dei processi salva l'indirizzo di questo array durante la system call per poterlo poi riusare durante la gestione dell'interrupt.

- startNstop: 
    in questa variabile viene salvato il TOD al momento del lancio di un nuovo processo. Nel momento in cui il processo viene interrotto per un eccezione la funzione UpdateCPUtime utilizza il valore salvato in startNstop per calcolare il tempo di utilizzo del processore e aggiornare il relativo campo del processo.

- is_waitin:
    quando viene conclusa la gestione di un interrupt ci sono due scenari possibili, l'interrupt è arrivato durante l'esecuzione di un processo e dunque il controllo va ritornato ad esso oppure non c'erano processi in esecuzione e dunque il controllo va ritornato allo scheduler. (fatta eccezione per PLT_interrupt_handler che richiama sempre lo scheduler)
    Utilizziamo questa variabile per stabilire quale delle due operazioni dobbiamo effettuare, prima di effettuare una WAIT lo scheduler setta is_waiting a true. Quandola gestione di un interrupt termina se is_waiting è uguale a true la variabile viene settata a false e viene invocato lo scheduler, altrimenti si esegue un LDST sul processo che era in esecuzione.  

sem_terminal


interrupt generale/terminale

calcolo device address



