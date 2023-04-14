#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <pcb.h>
#include <ash.h>
#include <ns.h>

int process_count; /* numero processi attivi */
int soft_block_count; /* conteggio processi bloccati per I/O o timer request*/

LIST_HEAD(readyQ); /* lista processi ready */
pcb_t* current_process; /* puntatore al processo in esecuzione */

int sem_processor_local_timer;
int sem_interval_timer;
int sem_disk[8];
int sem_tape[8];
int sem_network[8];
int sem_printer[8];
int sem_terminal[16];

void uTLB_RefillHandler () {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST ((state_PTR) 0x0FFFF000);
}

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
    sem_processor_local_timer=0;
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
    passupvector_t *passUpVect;
    passUpVect->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passUpVect->tlb_refill_stackPtr = (memaddr) 0x20001000;
    passUpVect->exception_handler = (memaddr) FUNZIONE_DA_IMPLEMENTARE;
    passUpVect->exception_stackPtr = (memaddr) 0x20001000; //stesso indirizzo ??

    //PUNTO 5 NON HO CAPITO
    LDIT(100000);     

    //PUNTO 6 
    pcb_t *primoProc = allocPcb();
    insertProcQ(&readyQ, &primoProc);
    process_count+=1;
    //bisogna finire l'inizializzazione del processo, non ho capito :(

}