#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <pcb.h>
#include <ash.h>
#include <ns.h>
#include <scheduler.c>
#include <initial.c>
#include <pandos_const.h>


void uTLB_RefillHandler () {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t *)0x0FFFF000);
}


void foobar() {
    /* usare i registri grp, che sono i registri a0, ..., a3 */
    /* prendere a0 per fare lo switch*/
    
    switch (a0)
    {
    case CREATEPROCESS:
        
        break;
        
    case TERMPROCESS:

        break;
    
    case PASSEREN:
        
        break;
        
    case VERHOGEN:

        break;
    
    case IOWAIT:
        
        break;
        
    case GETTIME:

        break;
    
    case GETSUPPORTPTR:
        
        break;
        
    case TERMINATE:

        break;
    default:
        break;
    }
}

