#include <umps3/umps/libumps.h>
#include <pcb.h>
#include <ash.h>
#include <ns.h>

int process_count; /* numero processi attivi */
int soft_block_count; /* conteggio processi bloccati per I/O o timer request*/

LIST_HEAD(readyQ); /* lista processi ready */
pcb_t* current_process; /* puntatore al processo in esecuzione */

int sem_interval_timer;
int sem_disk[8];
int sem_tape[8];
int sem_network[8];
int sem_printer[8];
int sem_terminal[16];


int main() {

/* inizializzazione strutture dati */
    initPcbs();
    initASH();
    initNamespaces();
    
    mkEmptyProcQ(&readyQ);

    process_count = 0;
    soft_block_count = 0;
    current_process = NULL;

/* inizializzazione semafori */
    sem_interval_timer=0;

    for( int i=0; i<8; i++) {
        sem_disk[i]=0;
        sem_tape[i]=0;
        sem_network[i]=0;
        sem_printer[i]=0;
        sem_terminal[i]=0;
        sem_terminal[8+i]=0;
    }

/* popolazione pass up vector */
    uTLB_RefillHandler();
    setTIMER(100);
}