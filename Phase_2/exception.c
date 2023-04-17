#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <pcb.h>
#include <ash.h>
#include <ns.h>
#include <scheduler.c>
#include <initial.c>
#include <pandos_const.h>

/* come process id usiamo un intero che aumenta 
    e basta (no caso reincarazione)*/
int pid = 0;

void uTLB_RefillHandler () {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t *)0x0FFFF000);
}


void foobar() {
    /* usare i registri grp, che sono i registri a0, ..., a3 */
    /* prendere a0 per fare lo switch*/

    switch (reg_a0)
    {
    case CREATEPROCESS:
        SYS_create_process(reg_a1, reg_a2, reg_a3);
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

void SYS_create_process(state_t *statep, support_t *supportp, nsd_t *ns)
{
    pcb_t *newProc = allocPcb();
    if ( newProc!=0) {
        newPorc->p_parent = current_process;
        newProc->p_s = *statep;
        if ( addNamspace(&newProc, &ns))
    }
    else {
        reg_v0 = -1;
    }
}