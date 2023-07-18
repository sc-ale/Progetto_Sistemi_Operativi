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
    tra i parametri della system call DoIO l'utente passa l'indirizzo di un array dentro il quale scrive i comandi da dare al device, al completamento dell'operazione lo status del device va copiato in questo array.  
    Il campo IOValues che abbiamo aggiunto al descrittore dei processi salva l'indirizzo di questo array durante la system call per poterlo poi riusare durante la gestione dell'interrupt.

- __startNstop__: 
    in questa variabile viene salvato il TOD al momento del lancio di un nuovo processo.  
    Nel momento in cui il processo viene interrotto per un eccezione la funzione UpdateCPUtime utilizza il valore salvato in startNstop per calcolare il tempo di utilizzo del processore da parte dell'ultimo processo in esecuzione, e aggiornare cosi' il relativo campo p_time.

- __is_waiting__:
    quando viene conclusa la gestione di un interrupt ci sono due scenari possibili:
    - L'interrupt e' arrivato durante l'esecuzione di un processo e dunque il controllo va ritornato ad esso 
    - Non c'erano processi in esecuzione e dunque il controllo va ritornato allo scheduler. (fatta eccezione per PLT_interrupt_handler che richiama sempre lo scheduler)  
    
    Utilizziamo la variabile is_waiting per stabilire quale delle due operazioni si debba effettuare.  
    Prima di entrare nello stato WAIT lo scheduler imposta is_waiting a true. Quando la gestione di un interrupt termina, se is_waiting è true allora la variabile viene settata a false e viene invocato lo scheduler, altrimenti si esegue un LDST sul processo che era in esecuzione.  

sem_terminal


interrupt generale/terminale

calcolo device address


## Compilazione 
                                            
Per compilare il progetto si utilizza il comando  `make`.  
Per eliminare i file creati si adopera il comando `make clean`.

I file di umps sono situati nella cartella "machine".  
Nel caso in cui si volesse realizzare una nuova configurazione su umps3 bisognerà selezionare la directory di "machine".

