#ifndef SCHEDULER_C 
#define SCHEDULER_C

#include "scheduler.h"
// necessario controllare se l'interrupt è stato chiamata durante
// l'esecuzione di un processo o durante una wait

void scheduling(){
    /* readyQ non vuota */
    if (!emptyProcQ(&readyQ))
    {
        current_process = removeProcQ(&readyQ);
        setTIMER(TIMESLICE);

        STCK(current_process->istante_Lancio_Blocco); 
        /* per salvarsi il momento in cui viene lanciato */
        
        LDST((STATE_PTR)&current_process->p_s);
    }
    else if(process_count==0)
    {
        aaa_readyQ_vuota();
        HALT();
    }
    else if(process_count>0 && soft_block_count>0)
    {
        /* the Scheduler must first set the Status register
            to enable interrupts and either disable the PLT
        The first interrupt that occurs after entering a 
        Wait State should not be for the PLT. */
        setSTATUS((IECON | IMON) & (~TEBITON));
        //setTIMER(NEVER);
        is_waiting = true;
        /* fare qualcosa con il PLT*/
        WAIT();
        aaa_readyQ_vuota();
        //quando esce dal wait dove va?
    }
    else if (process_count>0 && soft_block_count==0)
    {
        PANIC();
    }
}
#endif
