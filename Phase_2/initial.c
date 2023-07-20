#ifndef INITIAL_C
#define INITIAL_C
#include "initial.h"

/* -- Dichiarazione variabili globali -- */

int process_count;          /* numero processi attivi */
int soft_block_count;       /* numero processi bloccati per I/O o timer request*/

LIST_HEAD(readyQ);          /* lista processi ready */
pcb_t* current_process;     /* puntatore al processo in esecuzione */

bool is_waiting;            /* indica se lo scheduler e' in WAIT */

/* -- Semafori dei device e interval timer -- */
int sem_interval_timer;
int sem_disk[8];
int sem_tape[8];
int sem_network[8];
int sem_printer[8];
int sem_terminal[16];

int main() {
    /* -- Popolazione pass up vector -- */
    passupvector_t *passUpVect = (passupvector_t*) PASSUPVECTOR;
    passUpVect->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passUpVect->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    passUpVect->exception_handler = (memaddr) exception_handler;
    passUpVect->exception_stackPtr = (memaddr) KERNELSTACK; 

    /* -- Inizializzazione strutture dati prima fase -- */
    initPcbs();
    initASH();
    initNamespaces();

    /* -- Inizializzazione variabili del kernel -- */
    mkEmptyProcQ(&readyQ);
    process_count = 0;
    soft_block_count = 0;
    current_process = NULL;
    is_waiting = false;

    /* -- Inizializzazione semafori -- */
    sem_interval_timer=0;
    for(int i=0; i<8; i++) {
        sem_disk[i]=0;
        sem_tape[i]=0;
        sem_network[i]=0;
        sem_printer[i]=0;
        sem_terminal[i]=0;
        sem_terminal[8+i]=0;
    }

    /* Load interval timer */
    LDIT(PSECOND);     

    /* -- Creazione e inizializzazione del primo processo -- */
    pcb_t *primoProc = allocPcb();
    insertProcQ(&readyQ, primoProc);
    process_count+=1;
    primoProc->p_time = 0;
    primoProc->p_semAdd = NULL;
    primoProc->p_supportStruct = NULL;
    primoProc->p_pid = 1;
    primoProc->p_s.status |= (IEPON | IMON | TEBITON);
    primoProc->p_s.pc_epc = (memaddr) test;
    primoProc->p_s.reg_t9 = (memaddr) test;
    RAMTOP(primoProc->p_s.reg_sp);

    scheduling();
    return 0;
}

#endif