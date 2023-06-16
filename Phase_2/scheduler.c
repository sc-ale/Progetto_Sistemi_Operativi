#ifndef SCHEDULER_C 
#define SCHEDULER_C

#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <pandos_types.h>
#include <pandos_const.h>
#include "pcb.h"
#include "ash.h"
#include "ns.h"



extern int process_count; /* numero processi attivi */
extern int soft_block_count; /* conteggio processi bloccati per I/O o timer request*/
extern struct list_head readyQ;
extern pcb_t* current_process; /* puntatore al processo in esecuzione */
extern int sem_processor_local_timer;
extern int sem_interval_timer;
extern int sem_disk[8];
extern int sem_tape[8];
extern int sem_network[8];
extern int sem_printer[8];
extern int sem_terminal[16];
extern int sem_interval_timer;
extern void *memcpy(void *dest, const void *src, unsigned int n);


void scheduling(){
    while (true) {
        /* readyQ non vuota */
        if (!emptyProcQ(&readyQ))
        {
            current_process = removeProcQ(&readyQ);
            setTIMER(TIMESLICE);

            STCK(current_process->istante_Lancio_Blocco); 
            /* per salvarsi il momento in cui viene lanciato */
            
            LDST(&current_process->p_s);
        }
        else if(process_count==0)
        {
            HALT();
        }
        else if(process_count>0 && soft_block_count>0)
        {
            /* the Scheduler must first set the Status register
             to enable interrupts and either disable the PLT
            The first interrupt that occurs after entering a 
            Wait State should not be for the PLT. */
            setSTATUS((getSTATUS() ^ TEBITON) | IEPON);
            /* fare qualcosa con il PLT*/
            WAIT();
        }
        else if (process_count>0 && soft_block_count==0)
        {
            PANIC();
        }
        
    }
}
#endif
