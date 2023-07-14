#ifndef SCHEDULER_C 
#define SCHEDULER_C
#include "scheduler.h"

void scheduling() {
    if (!emptyProcQ(&readyQ)) {
        /* C'e' almeno un processo pronto ad essere eseguito */
        current_process = removeProcQ(&readyQ);
        setTIMER(TIMESLICE);

        /* Salvataggio del momento in cui viene lanciato */
        STCK(current_process->istante_Lancio_Blocco); 
        /* Caricamento del processo */
        LDST((STATE_PTR)&current_process->p_s);
    } else if (process_count==0) {
        HALT();
    } else if (process_count>0 && soft_block_count>0) {
        is_waiting = true;
        
        /* Disattivazione dell'interrupt del PLT */
        setSTATUS((IECON | IMON) & (~TEBITON));
        WAIT();
    } else if (process_count>0 && soft_block_count==0) {
        PANIC();
    }
}

#endif