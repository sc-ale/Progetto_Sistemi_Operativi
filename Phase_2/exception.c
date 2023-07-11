#ifndef EXCEPTION_C 
#define EXCEPTION_C
#include "exception.h"

/* CONTROLLARE LA SEZIONE 3.5.12 */
void foobar()
{
    updateCPUtime();
    bios_State = (state_t*) BIOSDATAPAGE;
    /* fornisce il codice del tipo di eccezione avvenuta */
    switch (CAUSE_GET_EXCCODE(bios_State->cause))
    {
    case 0:
        interrupt_handler();
        break;
    case 1 ... 3:
        passup_ordie(PGFAULTEXCEPT);
        break;
    case 4 ... 7:
        passup_ordie(GENERALEXCEPT);
        break;
    case 9 ... 12:
        passup_ordie(GENERALEXCEPT);
        break;
    case 8:
        syscall_handler();
    default:
        break;
    }
}

void updateCPUtime(){
    cpu_t momento_attuale;
    STCK(momento_attuale);
    current_process->p_time += momento_attuale - current_process->istante_Lancio_Blocco;
}

/*siccome perTLB , ProgramTrap e SYSCALL > 11 bisogna effettuare PASS UP OR DIE avrebbe senso creare una funzione*/
// DA RIGUARDARE 3.7
void passup_ordie(int INDEX)
{
    if (current_process->p_supportStruct == NULL) {
        SYS_terminate_process(0);
    }
    else {
        current_process->p_supportStruct->sup_exceptState[INDEX] = *(state_t*) BIOSDATAPAGE;
        context_t exceptContext = current_process->p_supportStruct->sup_exceptContext[INDEX];
        LDCXT(exceptContext.stackPtr, exceptContext.status, exceptContext.pc);
    }
}

/* Per le sys 3, 5, 7 servono delle operazioni in più, sezione 3.5.13 */
void syscall_handler()
{   
    if(!Check_Kernel_mode()) {
        passup_ordie(GENERALEXCEPT);
    }
    UPDATE_PC;  //evita i loop nelle syscall

    switch (bios_State->reg_a0)
    {
    case CREATEPROCESS:
        SYS_create_process((state_t*)bios_State->reg_a1, (support_t*)bios_State->reg_a2, (nsd_t*)bios_State->reg_a3);
        break;

    case TERMPROCESS:
        SYS_terminate_process((int)bios_State->reg_a1);
        break;

    case PASSEREN:
        SYS_Passeren((int*)bios_State->reg_a1);
        break;

    case VERHOGEN:
        SYS_Verhogen((int*)bios_State->reg_a1);
        break;

    case DOIO:
        SYS_Doio((int*)bios_State->reg_a1, (int*)bios_State->reg_a2);
        break;

    case GETTIME:
        SYS_Get_CPU_Time();
        break;

    case CLOCKWAIT:
        SYS_Clockwait();
        break;

    case GETSUPPORTPTR:
        SYS_Get_Support_Data();
        break;

    case GETPROCESSID:
        SYS_Get_Process_Id((int)bios_State->reg_a1);
        break;

    case GETCHILDREN:
        SYS_Get_Children((int*)bios_State->reg_a1, (int)bios_State->reg_a2);
        break;

    default:
        passup_ordie(GENERALEXCEPT);
        break;
    }
    /* non verrà eseguito se prima sono state seguite delle sys bloccanti */
    LDST(bios_State);
}

/* operazioni comuni per le sys bloccanti, l'inserimento nella ASL del current
 process viene fatto all'interno delle sys*/


/* Crea un nuovo processo come figlio del chiamante. Il primo parametro contiene lo stato
 che deve avere il processo. Se la system call ha successo il valore di ritorno (nel registro reg_v0)
 è il pid creato altrimenti è -1. supportp e’ un puntatore alla struttura di supporto del processo.
 Ns descrive il namespace di un determinato tipo da associare al processo, senza specificare
il namespace (NULL) verra’ ereditato quello del padre.*/
void SYS_create_process(state_t *statep, support_t *supportp, nsd_t *ns)
{
    pcb_t *newProc = allocPcb();
    pid_start++;

    if (newProc != NULL)
    {
        process_count++; /* stiamo aggiunge un nuovo processo tra quelli attivi */

        /* newProc sarà il figlio di current_process e sarà disponibile nella readyQ*/
        insertChild(current_process, newProc);
        insertProcQ(&readyQ, newProc);
        
        newProc->p_s = *statep;
        newProc->p_supportStruct = supportp;
        newProc->p_pid = pid_start; /* assegniamo il pid */
        newProc->p_time = 0;

        if (!addNamespace(newProc, ns))
        {                                       
            /* deve ereditare il ns dal padre */
            for (int i=0; i<NS_TYPE_MAX; i++) {
                nsd_t* tmpNs = getNamespace(current_process, i);
                addNamespace(newProc, tmpNs);
            }
        }

        UPDATE_BIOSSTATE_REGV0(newProc->p_pid);
    }
    else
    { /* non ci sono pcb liberi */
        UPDATE_BIOSSTATE_REGV0(-1);
    }
}

/* Termina il processo con identificativo pid e tutti suoi figli
 (e figli dei figli...) se pid è 0 allora termina il processo corrente */
void SYS_terminate_process(int pid)
{
    terminate_family2(pid);
    scheduling();
}


/* ritorna il pcb con pid dato */
pcb_t* getProcByPid(int pid) {
    pcb_t* proc2rtrn = NULL;
    /* verifico che il processo con p_pid == pid sia nella readyQ o su un semaforo*/
    if( (proc2rtrn = getProcInHead(pid, &readyQ)) == NULL) {
        /* non è in readyQ, quindi deve essere su qualche semaforo */
        aaa_procSEM();
        proc2rtrn = getProcByPidOnSem(pid);
    }
    if (proc2rtrn == NULL)  /* errore */
        aaa_pid_errato();

    return proc2rtrn;
}

/* funzione di fre, per vedere se il problema è qui
    - NON CE NE SONO
*/
void terminate_family2(int pid) {
    process_count--;                                                                   
    pcb_t* proc = (pid==0||pid==current_process->p_pid)?current_process:getProcByPid(pid);
    outChild(proc);                                                                         
    if(proc->p_semAdd!=NULL){                                                                   
        int * tmpSem = proc->p_semAdd;                                                          
        outBlocked(proc);                                                       
        if (is_sem_device_or_int(tmpSem)) {
            soft_block_count--;
        }
    }else if (proc!=current_process){                                          
        outProcQ(&readyQ, proc);                                                            
    }

    while(!emptyChild(proc)){                                                                  
        pcb_t* firstChild = list_first_entry(&proc->p_child,struct pcb_t,p_child);          
        removeChild(proc);                                                                      
        terminate_family2(firstChild->p_pid);                                                
    }
    freePcb(proc);                                                                     
    if(current_process==proc){                                                                   
        scheduling();
    }                           
}


/* Uccide un processo e tutta la sua progenie (NON I FRATELLI DEL PROCESSO CHIAMATO) */
void terminate_family(int pid)
{
    aaa_InsertMalePAS();
    pcb_t*Proc2Delete = (pid == 0 || current_process->p_pid == pid) ? current_process: getProcByPid(pid);
    outChild(Proc2Delete); /* Proc2delete staccato dal padre */
    
    while (!emptyChild(Proc2Delete))
    {
        /* se ha dei figli richiama la terminate_family */
        pcb_t *firstChild = list_first_entry(&Proc2Delete->p_child, struct pcb_t, p_child);
        removeChild(Proc2Delete);
        terminate_family(firstChild->p_pid);
    }

    kill_process(Proc2Delete);
}

bool is_sem_device_or_int(int* addSem) {
    return (addSem == &sem_interval_timer || addSem == sem_disk || addSem == sem_network || addSem == sem_printer || addSem == sem_tape || addSem == sem_terminal);
}

void kill_process(pcb_t *ptrn)
{
    process_count--;
    if (ptrn->p_semAdd != NULL)
    {   
        int * tmpSem = ptrn->p_semAdd;  
        /* processo bloccato su un semaforo */
        outBlocked(ptrn);
        if (is_sem_device_or_int(tmpSem)) {
            soft_block_count--;
        }
    }
    ptrn->p_pid = 0;
    freePcb(ptrn);
}

/* Operazione di richiesta di un semaforo binario.
 Il valore del semaforo è memorizzato nella variabile di tipo intero passata per indirizzo.
 L’indirizzo della variabile agisce da identificatore per il semaforo */
void SYS_Passeren(int *semaddr)
{
    
    /* dobbiamo usare la hash dei semafori attivi */
    // int pid_current = current_process->p_pid;
    if (*semaddr == 0)
    {
        /* aggiungere current_process nella coda dei
         processi bloccati da una P e sospenderlo*/
        //int inserimento_avvenuto =  Questa variabile non la usiamo?
        aaaBreakTest();
        aaaTest_variable = current_process->p_semAdd;
        if (insertBlocked(semaddr, current_process) == TRUE) {
            aaa_InsertMalePAS();
            aaaTest_variable = current_process->p_semAdd;
            aaa_val_di_Test_variabile = *(current_process->p_semAdd);
        }

        
        /* se inserimento_avvenuto è 1 allora non è stato possibile allocare 
         un nuovo SEMD perché la semdFree_h è vuota o perché current process ha già un sem */
        
        SAVESTATE;
        scheduling();
    }
    else if (headBlocked(semaddr) != NULL)
    { /* se la coda dei processi bloccati da V non è vuota*/
        /* risvegliare il primo processo che si era bloccato su una V */
        pcb_t *wakedProc = removeBlocked(semaddr);
        insertProcQ(&readyQ, wakedProc);
    }
    else
    {
        *semaddr=0;
    }
}

/* Operazione di rilascio di un semaforo binario la cui chiave è il valore puntato da semaddr */
void SYS_Verhogen(int *semaddr)
{
    //int pid_current = current_process->p_pid;
    if (*semaddr == 1)
    {
        /* aggiungere current_process nella coda dei
         processi bloccati da una V e sospenderlo*/
        
        aaaBreakTest();
        aaaTest_variable = current_process->p_semAdd;
        if (insertBlocked(semaddr, current_process) == TRUE) {
            aaa_InsertMaleVER();
            aaaTest_variable = current_process->p_semAdd;
        }
        /* se inserimento_avvenuto è 1 allora non è stato possibile allocare un nuovo SEMD perché la semdFree_h è vuota */

        /* chiamata allo scheduler, non so si può far direttamente così */
        SAVESTATE;
        scheduling();
    }
    else if (headBlocked(semaddr) != NULL)
    { /*Se la coda dei processi bloccati non è vuota*/
        /* risvegliare il primo processo che si era bloccato su una P */
        pcb_t *wakedProc = removeBlocked(semaddr);
        aaaTest_variable = wakedProc;
        insertProcQ(&readyQ, wakedProc); 
    }
    else
    {
        *semaddr=1;
    }
}

void OPERAZIONICOMUNIDOIO123(int *cmdAddr, int *cmdValues, int *sem, int devreg) {
    int devNo;
    for(int i=0; i<4; i++){
        cmdAddr[i] = cmdValues[i];
    }
    /*Calcola il device giusto e esegui una P sul suo semaforo*/
    devNo = devreg % 8;
    UPDATE_BIOSSTATE_REGV0(0);
    SYS_Passeren(&sem[devNo]);
}

/* Effettua un’operazione di I/O. CmdValues e’ un vettore di 2 interi
 (per I terminali) o 4 interi (altri device).
– La system call carica I registri di device con i valori di CmdValues
    scrivendo il comando cmdValue nei registri cmdAddr e seguenti,
    e mette in pausa il processo chiamante fino a quando non si e’ conclusa.
– L’operazione è bloccante, quindi il chiamante viene sospeso sino
    alla conclusione del comando. Il valore ritornato deve essere
    zero se ha successo, -1 in caso di errore. Il contenuto del registro di
    status del dispositivo potra’ essere letto nel corrispondente elemento
    di cmdValues.
At the completion of the I-O operation the device register values are
copied back in the cmdValues array
*/
void SYS_Doio(int *cmdAddr, int *cmdValues)
{
    /* chiamare update_PC_SYS_non_bloccanti(); */
        /* Mappa i registri dei device da 0 a 39*/
    int devreg = ((memaddr)cmdAddr - DEV_REG_START) / DEV_REG_SIZE;
    int devNo;
    soft_block_count++;
    current_process->IOvalues = cmdValues;
    switch (devreg / 8)
    {
    case 0:
        aaaCASO_0_test();
        OPERAZIONICOMUNIDOIO123(cmdAddr, cmdValues, sem_disk, devreg);
        break;
    case 1:
        aaaCASO_1_test();
        OPERAZIONICOMUNIDOIO123(cmdAddr, cmdValues, sem_tape, devreg);
        break;
    case 2: 
        aaaCASO_2_test();
        OPERAZIONICOMUNIDOIO123(cmdAddr, cmdValues, sem_network, devreg);
        break;
    case 3:
        aaaCASO_3_test();
        OPERAZIONICOMUNIDOIO123(cmdAddr, cmdValues, sem_printer, devreg);;
        break;
    case 4:
        aaaCASO_4_test();
        /*I registri dei terminali sono divisi in due (ricezione / trasmissione), 
            facendo l'indirizzo modulo 16 capiamo se siamo all'inizio del registro 
            (e quindi ricezione)
            oppure a metà del registro (e quindi trasmissione)*/
        for(int i=0; i<2; i++){
            cmdAddr[i] = cmdValues[i];
        }
        //is_terminal = true;
        devNo = *cmdAddr%16 == 0 ? devreg%8 : (devreg%8)+8;
        UPDATE_BIOSSTATE_REGV0(0);
        SYS_Passeren(&sem_terminal[devNo]);
        break;
    default:
        soft_block_count--;
        UPDATE_BIOSSTATE_REGV0(-1);
        break;
    }
}

/* Restituisce il tempo di utilizzo del processore del processo in esecuzione*/
void SYS_Get_CPU_Time()
{
    /* Hence SYS6 should return the value in the Current Process’s p_time PLUS
     the amount of CPU time used during the current quantum/time slice.*/
    UPDATE_BIOSSTATE_REGV0(current_process->p_time);
}

/*
Equivalente a una Passeren sul semaforo dell’Interval Timer.
– Blocca il processo invocante fino al prossimo tick del dispositivo.
*/
void SYS_Clockwait()
{
    if(sem_interval_timer == 0) {
    /* aggiungere current_process nella coda dei processi bloccati da una P e sospenderlo*/
    insertBlocked(&sem_interval_timer, current_process);
    soft_block_count++;
    }
    /* se inserimento_avvenuto è 1 allora non è stato possibile allocare un nuovo SEMD perché la semdFree_h è vuota */

    SAVESTATE;
    scheduling();
}

/* Restituisce un puntatore alla struttura di supporto del processo corrente,
 ovvero il campo p_supportStruct del pcb_t.*/
void SYS_Get_Support_Data()
{
    UPDATE_BIOSSTATE_REGV0(current_process->p_supportStruct);
    //return current_process->p_supportStruct;
}

/* Restituisce l’identificatore del processo invocante se parent == 0,
 quello del genitore del processo invocante altrimenti.
 Se il parent non e’ nello stesso PID namespace del processo figlio,
 questa funzione ritorna 0 (se richiesto il pid del padre)! */
void SYS_Get_Process_Id(int parent)
{
    if (parent == 0)
    {
        UPDATE_BIOSSTATE_REGV0(current_process->p_pid);
    }
    else
    { /* dobbiamo restituire il pid del padre, se si trovano nello stesso namespace */
        /* assumiamo che il processo corrente abbia un padre (?) */
        // NON CREDO SI POSSA FARE QUESTA ASSUNZIONE

        nsd_t *parentNs = getNamespace(current_process->p_parent, NS_PID);
        aaaTest_variable = parentNs;
        aaa_val_di_Test_variabile = &parentNs;
        /* se current_process e il processo padre non sono nello stesso namespace restituisci 0 */
        int pid2save = (parentNs != getNamespace(current_process, NS_PID)) ? 0 : current_process->p_parent->p_pid;
        aaaBreakTest();
        aaaTest_variable = pid2save;
        UPDATE_BIOSSTATE_REGV0(pid2save);
    }
}

/* Ritorna il numero di pid dei figli che appartengono allo stesso ns del chiamante e
 li salva nell'array children di dimensione size */
void SYS_Get_Children(int *children, int size)
{
    int num = 0;
    if(!emptyChild(current_process)) 
    {                                              
        pcb_t* firstChild = list_first_entry(&current_process->p_child,struct pcb_t,p_child);    
        nsd_t* currNs = getNamespace(current_process, NS_PID);                               
        if (currNs == getNamespace(firstChild, NS_PID)){                                    
            if (num < size)                                                       
                children[num] = firstChild->p_pid; 
            num++;                                                                    
        }
        
        /* è necessario loopare sui fratelli in modo distinto siccome 
         firstChild è l'elemento sentinella, quindi non vede se stesso */
        struct pcb_t* currPcb = NULL;
        list_for_each_entry(currPcb,&firstChild->p_sib,p_sib){                        
            if (currNs == getNamespace(currPcb, NS_PID)){   
                if (num < size)
                    children[num] = currPcb->p_pid;                            
                num++;
            }
        }
    }
    UPDATE_BIOSSTATE_REGV0(num);
    
}

/* Per determinare se il processo corrente stava eseguento in kernel o user mode,
 dobbiamo esaminare lo Status register in the saved exception state.
 In particolare, dobbiamo esaminare la versione precedente del KU bit (KUp) siccome
 sarà avvenuta una stack push sul KU/IE stacks in the statusa register prima
 che lo stato di eccezione fosse salvato*/
int Check_Kernel_mode()
{
    unsigned mask;
    mask = ((1 << 1) - 1) << STATUS_KUp_BIT;
    unsigned int bit_kernel = bios_State->status & mask;
    /* ritorna vero se il processo era in kernel mode, 0 in user mode*/
    return (bit_kernel == 0) ? TRUE : FALSE;
}

#endif