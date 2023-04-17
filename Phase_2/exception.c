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
int pid_start = 0;

void uTLB_RefillHandler () 
{
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t *)0x0FFFF000);
}


void foobar() 
{
    /* usare i registri grp, che sono i registri a0, ..., a3 */
    /* prendere a0 per fare lo switch*/

    switch (reg_a0)
    {
    case CREATEPROCESS:
        SYS_create_process(reg_a1, reg_a2, reg_a3);
        break;
        
    case TERMPROCESS:
        SYS_terminate_process(reg_a1);
        break;
    
    case PASSEREN:
        SYS_Passeren(reg_a1);
        break;
        
    case VERHOGEN:
        SYS_Verhogen(reg_a1);
        break;
    
    case IOWAIT:
        
        break;
        
    case GETTIME:
        SYS_Get_CPU_Time();
        break;

    case CLOCKWAIT:
        break;
    
    case GETSUPPORTPTR:
        
        break;
        
    case TERMINATE:

        break;
    default:
        break;
    }
}

/* Crea un nuovo processo come figlio del chiamante. Il primo parametro contiene lo stato
 che deve avere il processo. Se la system call ha successo il valore di ritorno (nel registro reg_v0)
 è il pid creato altrimenti è -1. supportp e’ un puntatore alla struttura di supporto del processo.
 Ns descrive il namespace di un determinato tipo da associare al processo, senza specificare
il namespace (NULL) verra’ ereditato quello del padre.*/
void SYS_create_process(state_t *statep, support_t *supportp, nsd_t *ns)
{
    pcb_t *newProc = allocPcb();

    if (newProc!=NULL) {
        /* newProc sarà il figlio di current_process e sarà disponibile nella readyQ*/
        insertChild(&current_process, &newProc);
        insertProcQ(&readyQ, newProc);
        newProc->p_s = *statep;
        newProc->p_supportStruct = supportp;

        if (!addNamespace(&newProc, &ns)) { /* deve ereditare il ns dal padre */
            newProc->namespaces = current_process->namespace;
        }

        newProc->p_pid = pid_start + 1; /* assegniamo il pid */
        newProc->p_time = 0;

        //process_count=+1; /* stiamo aggiunge un nuovo processo tra quelli attivi ? */

        reg_v0 = newProc->p_pid;
    }
    else { /* non ci sono pcb liberi */
        reg_v0--;
    }
}


/* Termina il processo con identificativo pid e tutti suoi figli
 (e figli dei figli...) se pid è 0 allora termina il processo corrente */
void SYS_terminate_process(int pid)
{
    pcb_t *Proc2Delete;
    if(pid == 0){
        Proc2Delete = current_process;
    } else{
        for(int i=0; i<MAXPROC; i++){
            if(pcbFree_table[i]->p_pid == pid){
                Proc2Delete = &pcbFree_table[i];
            }
        }
        return;
    }

    mass_Murder(Proc2Delete);
    return;
}

/* aggiunge tutti i fratelli del figlio in freePcb_h */
void mass_Murder(pcb_t *father){
    struct list_head *pos, *current = NULL;

    list_for_each_safe(pos, current, father->p_child->p_sib){
        pcb_t* temp = list_entry(pos, struct pcb_t, p_sib);
        mass_Murder(temp);
    }
    father-> p_pid = 0;
    freePcb(father);  
}

void terminate_family(pcb_t *ptrn)
{
    /* se ha dei figli richiama la funzione stessa */
    if(!emptyChild(ptrn)) {        
        terminate_family(ptrn->p_child);   
    }

    /* richiamo la funzione per i fratelli di ptrn */
    struct list_head *pos, *current = NULL;
    list_for_each_safe(pos, current, ptrn->p_sib) {
        pcb_t* temp = list_entry(pos, struct pcb_t, p_sib);
        terminate_family(temp);
    }

    /* uccido ptrn */
    ptnr->p_parent = NULL;
    list_del(&ptrn->p_list);
    list_del(&ptrn->p_child);
    list_del(&ptrn->p_sib);
    ptrn->p_semAdd = NULL;
    ptrn->p_pid = 0;
    freePcb(ptrn);
    
    process_count--;
    /* penso che dobbiamo controllare se tra i processi che eliminiamo
     ci sono dei processi bloccati e in quel caso diminuire il soft_block_count */
}


/* Operazione di richiesta di un semaforo binario.
 Il valore del semaforo è memorizzato nella variabile di tipo intero passata per indirizzo. 
 L’indirizzo della variabile agisce da identificatore per il semaforo */
void SYS_Passeren(int *semaddr) 
{
    /* dobbiamo usare la hash dei semafori attivi */
    int pid_current = current_process->p_pid;
    if(*semaddr==0) {
        /* aggiungere current_process nella coda dei 
         processi bloccati da una P e sospenderlo*/
        int inserimento_avvenuto = insertBlocked(semaddr, current_process);
        /* se inserimento_avvenuto è 1 allora non è stato possibile allocare un nuovo SEMD perché la semdFree_h è vuota */

         /* chiamata allo scheduler, non so si può far direttamente così */
        scheduling();
    } else if( /* se la coda dei processi bloccati da V non è vuota*/(pcb_t* temp = headBlocked(semaddr))!=NULL) {
        /* risvegliare il primo processo che si era bloccato su una V */
        pcb_t* wakedProc = removeBlocked(semaddr);
    } else {
        semaddr--;
    }
}

/* Operazione di rilascio di un semaforo binario la cui chiave è il valore puntato da semaddr */
void SYS_Verhogen(int* semaddr)
{
    int pid_current = current_process->p_pid;
    if(*semaddr==1) {
        /* aggiungere current_process nella coda dei 
         processi bloccati da una V e sospenderlo*/
        int inserimento_avvenuto = insertBlocked(semaddr, current_process);
        /* se inserimento_avvenuto è 1 allora non è stato possibile allocare un nuovo SEMD perché la semdFree_h è vuota */

         /* chiamata allo scheduler, non so si può far direttamente così */
        scheduling();
    } else if( /* se la coda dei processi bloccati da P non è vuota*/(pcb_t* temp = headBlocked(semaddr))!=NULL) {
        /* risvegliare il primo processo che si era bloccato su una P */
        pcb_t* wakedProc = removeBlocked(semaddr);
    } else {
        semaddr++;
    }
}

SYS_Get_CPU_Time()
{
    reg_v0 = current_process->p_time;
}