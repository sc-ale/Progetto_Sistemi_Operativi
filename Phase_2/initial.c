#ifndef INITIAL_C
#define INITIAL_C

#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <pandos_types.h>
#include <pandos_const.h>
#include "pcb.h"
#include "ash.h"
#include "ns.h"

extern void test();
extern void scheduling();
extern void uTLB_RefillHandler();
extern void foobar();
extern void addokbuf();
extern void adderrbuf();

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

extern void *memcpy(void *dest, const void *src, unsigned int n);


int main() {

    addokbuf("inizio \n");
/* inizializzazione strutture dati */
    initPcbs();
    initASH();
    initNamespaces();
    addokbuf("initBene \n");
    
    mkEmptyProcQ(&readyQ);
    addokbuf("dopo mkEmptyProcQ \n");

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
    addokbuf("errore");
    passupvector_t *passUpVect;
    passUpVect->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passUpVect->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    passUpVect->exception_handler = (memaddr) foobar;
    passUpVect->exception_stackPtr = (memaddr) KERNELSTACK; //stesso indirizzo ??

    addokbuf("passUP ok \n");
    //PUNTO 5 NON HO CAPITO
    LDIT(PSECOND);     

    addokbuf("LDIT ok \n");

    //PUNTO 6 
    pcb_t *primoProc = allocPcb();
    insertProcQ(&readyQ, primoProc);
    process_count+=1;
    primoProc->p_time = 0;
    primoProc->p_supportStruct = NULL;

    // Sezione 2.3 di uMPS3 spiega questo registro, non ho trovato macro o altro
    // l'unica soluzione mi sembra assegnarli in modo diretto ma guardateci anche voi
    primoProc->p_s.status = (IEPON | IMON | TEBITON);



    //PC e SP settati modo 1
    primoProc->p_s.pc_epc = (memaddr) test;
    primoProc->p_s.reg_t9 = (memaddr) test;
    RAMTOP(primoProc->p_s.reg_sp);
    /*Macro che associa al chiamante l'inidirizzo RAMTOP*/

    addokbuf("RAM TOP OK \n");
    LDST(&primoProc->p_s);

    addokbuf("LDST ok \n");

    scheduling();

    return 0;
}


#endif