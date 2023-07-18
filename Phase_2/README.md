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

### initial.c : 

- Inizializza le variabili globali del nucleo e le varie strutture dati

- Crea e inizializza il primo processo

- Invoca lo scheduler

### scheduler.c :

Il ruolo dello scheduler e’ di decidere quale processo deve entrare in esecuzione

### exception.c :

Gestisce le eccezioni del sistema, identificando il tipo di eccezione avvenuta e richiamando l'handler appropriato per gestire l'eccezione specifica tra cui:

- Interrupt

- TLB Trap

- Program Trap

- Syscall

### interrupt.c :

Si occupa di determinare l'interrupt facendo distinzione tra:

- Processor Local Timer interrupt

- Interval Timer interrupt

- Device interrupt

E successivamente le affida ai vari gestori di interrupt.


## Scelte progettuali e Robe aggiunte

## Compilazione 
                                            
Per compilare il progetto bisogna utilizzare il comando  

```
make
```
e per eliminare i file creati con make si utilizza 
```
make clean
```

I file di umps sono situati nella cartella "machine". Nel caso si volesse realizzare una nuova configurazione su umps3 bisognerà selezionare la directory di "machine".

