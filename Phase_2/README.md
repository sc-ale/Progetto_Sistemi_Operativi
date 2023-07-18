# PANDOS PHASE 2

Nella seconda fase del progetto l'obiettivo è quello di realizzare il kernel del S.O. Panda+.  
Le funzionalità che deve gestire sono:

- Inizializzazione del sistema
- Scheduling dei processi
- Gestione delle eccezioni:
    - Interrupt
    - Syscall
    - Trap

## DIVISIONE FILE

I file principali della fase 2 sono divisi come segue:

- ### initial.c : 
    - Inizializza le variabili globali del nucleo e le varie strutture dati
    - Crea ed inizializza il primo processo
    - Invoca lo scheduler

- ### scheduler.c :
    Il ruolo dello scheduler e’ di gestire l'avvicendamento dei processi

- ### exception.c :
    Distingue il tipo di eccezione del sistema e la risolve attraverso il gestore opportuno.  
    In particolare, i tipi di eccezione possono essere:
    - Interrupt
    - TLB Trap
    - Program Trap
    - Syscall

- ### interrupt.c :
    Si occupa di determinare l'interrupt facendo distinzione tra:
    - Processor Local Timer interrupt
    - Interval Timer interrupt
    - Device interrupt  

    E successivamente le affida ai vari gestori di interrupt.


## Scelte progettuali

Variabili aggiunte

- __IOValues__:
    tra i parametri della system call DoIO l'utente passa l'indirizzo di un array dentro il quale scrive i comandi da dare al device, al completamento dell'operazione lo status del device va scritto in questo array. Il campo IOValues che abbiamo aggiunto al descrittore dei processi salva l'indirizzo di questo array durante la system call per poterlo poi riusare durante la gestione dell'interrupt.

- __startNstop__: 
    in questa variabile viene salvato il TOD al momento del lancio di un nuovo processo. Nel momento in cui il processo viene interrotto per un eccezione la funzione UpdateCPUtime utilizza il valore salvato in startNstop per calcolare il tempo di utilizzo del processore e aggiornare il relativo campo del processo.

- __is_waiting__:
    quando viene conclusa la gestione di un interrupt ci sono due scenari possibili, l'interrupt è arrivato durante l'esecuzione di un processo e dunque il controllo va ritornato ad esso oppure non c'erano processi in esecuzione e dunque il controllo va ritornato allo scheduler. (fatta eccezione per PLT_interrupt_handler che richiama sempre lo scheduler)
    Utilizziamo questa variabile per stabilire quale delle due operazioni dobbiamo effettuare, prima di effettuare una WAIT lo scheduler setta is_waiting a true. Quandola gestione di un interrupt termina se is_waiting è uguale a true la variabile viene settata a false e viene invocato lo scheduler, altrimenti si esegue un LDST sul processo che era in esecuzione.  

Semaforo per i terminali

- Mentre per gli altri dispositivi utilizziamo un array di 8 semafori (uno per ogni device), per i terminali abbiamo scelto di utilizzare un array di 16 semafori utilizzando i primi 8 (da 0 a 7) per la ricezione e gli ultimi 8 (da 8 a 15) per la trasmissione. In questo modo ciascun device n avrà i suoi rispettivi semafori alle posizioni n e n+8.

Interrupt handler per i device

- Data la diversità dei terminali dagli altri tipi di device abbiamo deciso di utilizzare due funzioni diverse per gestire i relativi interrupt: terminal_interrupt_handler per gli interrupt dei terminali e general_interrupt_handler per tutti gli altri device.

Calcolo device address

- Quando gestiamo la chiamata di una SYSCall DoIO per stabilire quale device l'utente sta richiedendo lo calcoliamo a partire dall'address fornito in input. 
    ```C
        int devReg = ((memaddr)cmdAddr - DEV_REG_START) / DEV_REG_SIZE;
        int typeDevice = devReg/8;
    ```
- Sottraendo l'indirizzo di partenza dei device register e dividendo per 16 (dimensione di ogni device register) otteniamo un valore compreso tra 0 e 39 (8 device di 5 tipi quindi 40 device in tutto).
Dividendo per 8 stabiliamo quale tipo di device stiamo cercando (da 0 a 4 rispettivamente disk, tape, network, printer e terminal).

## Compilazione 
                                            
Per compilare il progetto si utilizza il comando  `make`.  
Per eliminare i file creati si adopera il comando `make clean`.

I file di umps sono situati nella cartella "machine".  
Nel caso in cui si volesse realizzare una nuova configurazione su umps3 bisognerà selezionare la directory di "machine".

