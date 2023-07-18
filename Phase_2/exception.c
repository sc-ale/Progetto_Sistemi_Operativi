#ifndef EXCEPTION_C 
#define EXCEPTION_C
#include "exception.h"

void exception_handler() {
    updateCPUtime();
    bios_State = (state_t*) BIOSDATAPAGE;
    /* fornisce il codice del tipo di eccezione avvenuta */
    switch (CAUSE_GET_EXCCODE(bios_State->cause)) {

        case INTERRUPTEXC:
            interrupt_handler();
            break;

        case TLBEXCEPT:
            passup_ordie(PGFAULTEXCEPT);
            break;
        
        case PROGTRAP1:
            passup_ordie(GENERALEXCEPT);
            break;
        
        case SYSEXCEPTION:
            syscall_handler();
            break;
        
        case PROGTRAP2:
            passup_ordie(GENERALEXCEPT);
            break;

        default:
            passup_ordie(GENERALEXCEPT);
            break;
    }
}

void updateCPUtime(){
    cpu_t momento_attuale;
    STCK(momento_attuale);
    current_process->p_time += (momento_attuale - current_process->startNstop);
}

void passup_ordie(int INDEX) {
    if (current_process->p_supportStruct == NULL) {
        SYS_terminate_process(0);
    } else {
        current_process->p_supportStruct->sup_exceptState[INDEX] = *(state_t*) BIOSDATAPAGE;
        context_t exceptContext = current_process->p_supportStruct->sup_exceptContext[INDEX];
        LDCXT(exceptContext.stackPtr, exceptContext.status, exceptContext.pc);
    }
}

void syscall_handler() {   
    UPDATE_PC;
    if (!Check_Kernel_mode()) {
        passup_ordie(GENERALEXCEPT);
    } else {
        switch ((int)bios_State->reg_a0) {

            case CREATEPROCESS:
                SYS_create_process((state_t*)bios_State->reg_a1, (support_t*)bios_State->reg_a2, (nsd_t*)bios_State->reg_a3);
                break;

            case TERMPROCESS:
                SYS_terminate_process((int)bios_State->reg_a1);
                break;

            case PASSEREN:
                SYS_passeren((int*)bios_State->reg_a1);
                break;

            case VERHOGEN:
                SYS_verhogen((int*)bios_State->reg_a1);
                break;

            case DOIO:
                SYS_doio((int*)bios_State->reg_a1, (int*)bios_State->reg_a2);
                break;

            case GETTIME:
                SYS_get_CPU_time();
                break;

            case CLOCKWAIT:
                SYS_clockwait();
                break;

            case GETSUPPORTPTR:
                SYS_get_support_data();
                break;

            case GETPROCESSID:
                SYS_get_process_id((int)bios_State->reg_a1);
                break;

            case GETCHILDREN:
                SYS_get_children((int*)bios_State->reg_a1, (int)bios_State->reg_a2);
                break;

            default:
                passup_ordie(GENERALEXCEPT);
                break;
        }
        
        /* Il seguente codice non verrà eseguito se prima sono state seguite delle sys bloccanti */
        LDST(bios_State);
    }
}


static void SYS_create_process(state_t *statep, support_t *supportp, nsd_t *ns) {

    pcb_t *newProc = allocPcb();

    if (newProc != NULL) {

        pid_start++;
        process_count++; 

        /* Aggiunta del nuovo processo alla lista dei figli del chiamante e alla readyQ */
        insertChild(current_process, newProc);
        insertProcQ(&readyQ, newProc);
        
        /* -- Inizializzazione campi nuovo processo -- */
        newProc->p_s = *statep;
        newProc->p_supportStruct = supportp;
        newProc->p_pid = pid_start;
        newProc->p_time = 0;

        if (!addNamespace(newProc, ns)) {                                       
            /* Deve ereditare il ns dal padre */
            for (int i=0; i<NS_TYPE_MAX; i++) {
                nsd_t* tmpNs = getNamespace(current_process, i);
                addNamespace(newProc, tmpNs);
            }
        }

        UPDATE_BIOSSTATE_REGV0(newProc->p_pid);
    } else {
        /* Non ci sono pcb liberi */
        UPDATE_BIOSSTATE_REGV0(-1);
    }
}


static void SYS_terminate_process(int pid) {
    terminate_family(pid);
    scheduling();
}


pcb_t* getProcByPid(int pid) {
    /* Verifico che il processo con p_pid == pid sia nella readyQ o su un semaforo */
    pcb_t* proc2rtrn = NULL;
    if ((proc2rtrn = getProcInHead(pid, &readyQ)) == NULL) {
        /* Non è in readyQ, quindi deve essere su qualche semaforo */
        proc2rtrn = getProcByPidOnSem(pid);
    }
    return proc2rtrn;
}


void terminate_family(int pid) {
    pcb_t*Proc2Delete = (pid == 0 || current_process->p_pid == pid) ? current_process : getProcByPid(pid);
    /* Proc2delete staccato dal padre */
    outChild(Proc2Delete); 
    
    while (!emptyChild(Proc2Delete)) {
        /* Se ha dei figli richiama la terminate_family */
        pcb_t *firstChild = list_first_entry(&Proc2Delete->p_child, struct pcb_t, p_child);
        removeChild(Proc2Delete);
        terminate_family(firstChild->p_pid);
    }

    kill_process(Proc2Delete);
}


void kill_process(pcb_t *ptrn) {
    process_count--;
    if (ptrn->p_semAdd != NULL) {   
        int * tmpSem = ptrn->p_semAdd;  
        /* Processo bloccato su un semaforo */
        outBlocked(ptrn);
        if (IS_SEM_DEVICE_OR_INT(tmpSem)) {
            soft_block_count--;
        }
    } else if (ptrn!=current_process) {
        /* Processo nella readyQ */                                          
        outProcQ(&readyQ, ptrn);                                                            
    }
    ptrn->p_pid = 0;
    freePcb(ptrn);
}


static void SYS_passeren(int *semaddr) {
    if (*semaddr == 0) {
        /* Inserimento di current_process nella coda del semaforo semaddr */
        insertBlocked(semaddr, current_process);
        SAVESTATE;
        scheduling();
    } else if (headBlocked(semaddr) != NULL) { 
        /* Risveglia il primo processo bloccato su semaddr */
        pcb_t *wakedProc = removeBlocked(semaddr);
        insertProcQ(&readyQ, wakedProc);
    } else {
        *semaddr=0;
    }
}


void SYS_verhogen(int *semaddr) {
    if (*semaddr == 1) {
        /* Inserimento di current_process nella coda del semaforo semaddr */
        insertBlocked(semaddr, current_process);
        SAVESTATE;
        scheduling();
    } else if (headBlocked(semaddr) != NULL) {
        /* Risveglia il primo processo bloccato su semaddr */
        pcb_t *wakedProc = removeBlocked(semaddr);
        insertProcQ(&readyQ, wakedProc); 
    } else {
        *semaddr=1;
    }
}


static void SYS_doio(int *cmdAddr, int *cmdValues) {
    /* devReg mappa i registri dei device da 0 a 39 */
    int devReg = ((memaddr)cmdAddr - DEV_REG_START) / DEV_REG_SIZE;
    int typeDevice = devReg/8;
    
    soft_block_count++;
    current_process->IOvalues = cmdValues;

    if (typeDevice<0 || typeDevice>4) {
        soft_block_count--;
        UPDATE_BIOSSTATE_REGV0(-1);
    } else {
        general_doio(cmdAddr, cmdValues, devReg, typeDevice);
    }
}


void general_doio(int *cmdAddr, int *cmdValues, int devReg, int typeDevice) {
    int *sem2use = deviceType2Sem(typeDevice);
    int iterMax = (typeDevice == 4) ? 2:4;

    for (int i=0; i<iterMax; i++) {
        cmdAddr[i] = cmdValues[i];
    }
    /* Calcola il device specifico */
    int devNo;
    if (typeDevice == 4) {
        /* Il device è un terminale */
        devNo = (*cmdAddr%16 == 0) ? devReg%8 : (devReg%8)+8;
    } else {
        devNo = devReg % 8;
    }
    UPDATE_BIOSSTATE_REGV0(0);
    SYS_passeren(&sem2use[devNo]);
}


int* deviceType2Sem(int type) {
    int *sem2rtrn;
    /* Selezione del tipo di device */
    switch (type) {
        case 0:
            sem2rtrn = sem_disk;
            break;
        case 1:
            sem2rtrn = sem_tape;
            break;
        case 2:
            sem2rtrn = sem_network;
            break;
        case 3:
            sem2rtrn = sem_printer;
            break;
        case 4:
            sem2rtrn = sem_terminal;
            break;
        default:
            sem2rtrn = NULL;
            break;
    }
    return sem2rtrn;
}


static void SYS_get_CPU_time() {
    UPDATE_BIOSSTATE_REGV0(current_process->p_time);
}


static void SYS_clockwait() {
    /* Inserimento di current_process nella coda dei processi bloccati sull'interval timer */
    insertBlocked(&sem_interval_timer, current_process);
    soft_block_count++;
    SAVESTATE;
    scheduling();
}


static void SYS_get_support_data() {
    UPDATE_BIOSSTATE_REGV0(current_process->p_supportStruct);
}


static void SYS_get_process_id(int parent) {
    if (parent == 0) {
        UPDATE_BIOSSTATE_REGV0(current_process->p_pid);
    } else {
        /* Verifica che padre e figlio si trovino nello stesso namespace */
        nsd_t *parentNs = getNamespace(current_process->p_parent, NS_PID);
        int pid2save = (parentNs != getNamespace(current_process, NS_PID)) ? 0 : current_process->p_parent->p_pid;
        UPDATE_BIOSSTATE_REGV0(pid2save);
    }
}


static void SYS_get_children(int *children, int size) {
    int num = 0;
    if (!emptyChild(current_process)) {
        /* Controllo sul primo figlio */                                              
        pcb_t* firstChild = list_first_entry(&current_process->p_child,struct pcb_t,p_child);    
        nsd_t* currNs = getNamespace(current_process, NS_PID);                               
        if (currNs == getNamespace(firstChild, NS_PID)) {                                    
            if (num < size) {                                                       
                children[num] = firstChild->p_pid; 
            }
            num++;                                                                    
        }
        
        /* Controllo sui fratelli usando il primo figlio come sentinella */
        struct pcb_t* currPcb = NULL;
        list_for_each_entry(currPcb,&firstChild->p_sib,p_sib) {    
            if (currNs == getNamespace(currPcb, NS_PID)) {   
                if (num < size) {
                    children[num] = currPcb->p_pid;                            
                }
                num++;
            }
        }
    }
    UPDATE_BIOSSTATE_REGV0(num);
}


bool Check_Kernel_mode() {
    /* Il processo era in kernel mode se il bit in posizione KUp è settato a 0*/
    unsigned mask;
    mask = 1 << STATUS_KUp_BIT;
    unsigned int bit_kernel = bios_State->status & mask;
    /* 0 kernel mode 1 user */
    return (bit_kernel == 0) ? TRUE : FALSE;
}

#endif