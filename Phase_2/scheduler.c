#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <initial.c>
#include <pcb.h>
#include <ash.h>
#include <ns.h>

void scheduling(){
    while (true) {
        /* readyQ non vuota */
        if (!emptyProcQ(&readyQ))
        {
            current_process = removeProcQ(&readyQ);
            setTIMER(500);
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
            setSTATUS(0x/*save aiuto*/);
            /* fare qualcosa con il PLT*/
            WAIT();
        }
        else if (process_count>0 && soft_block_count==0)
        {
            PANIC();
        }
        
    }
}
