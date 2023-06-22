#ifndef INTERRUPT_C 
#define INTERRUPT_C

#include "interrupt.h"



void *memcpy(void *dest, const void *src, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
    //Questo return l'ho aggiunto per togliere il warning(non dovrebbe creare problemi
    // la funzione memcpy dovrebbe ritornare la locazione di memoria dove ha copiato)
    return dest;
} 

/* Restituisce la linea con interrupt in attesa con massima priorità. 
(Se nessuna linea è attiva ritorna 8 ma assumiamo che quando venga
 chiamata ci sia almeno una linea attiva) */
int Get_Interrupt_Line_Max_Prio (){
    unsigned int interrupt_pending = bios_State->cause & CAUSE_IP_MASK;
    /* così abbiamo solo i bit attivi da 8 a 15 del cause register */
    unsigned int intpeg_linee[8];
    for (int i=0; i<8; i++) {
        unsigned mask = ((1<<1)-1)<<(i+8);
        intpeg_linee[i] = mask & interrupt_pending;
    }
    /* intpeg_linee[i] indica se la linea i-esima è attiva */
    
    int linea=1; /* questo perché ignoriamo la linea 0*/
    while(linea<8) {
        if(intpeg_linee[linea]!=0) { 
            break;
        }
        linea++;
    }
    /* ritorniamo la quale linea è attiva */
    return linea;
}

/*
The interrupt exception handler’s first step is to determine which device
 or timer with an outstanding interrupt is the highest priority.
 Depending on the device, the interrupt exception handler will 
 perform a number of tasks.*/
void interrupt_handler()
{
    /* sezione 3.6.1 a 3.6.3*/
    switch (Get_Interrupt_Line_Max_Prio())
    {
    /* interrupt processor Local Timer */
    case 1:
        PLT_interrupt_handler();
        break;
    
    /* interrupt Interval Timer */
    case 2:
        IT_interrupt_handler();
        break;

    /* Disk devices */
    case DISKINT:
        general_interrupt_handler(DISKINT); /* passare la interrupt line*/
        break;
    
    /* Flash devices */
    case FLASHINT:
        general_interrupt_handler(FLASHINT); /* passare la interrupt line*/
        break;
    
    /* Network devices*/
    case NETWINT:
        general_interrupt_handler(NETWINT); /* passare la interrupt line*/
        break;

    /* Printer devices */
    case PRNTINT:
        general_interrupt_handler(PRNTINT); /* passare la interrupt line*/
        break;
    
    /* Terminal devices*/
    case TERMINT:
        terminal_interrupt_handler(TERMINT); /* passare la interrupt line*/
        break;

    default:
        break;
    }
   
}

//3.6.2
void PLT_interrupt_handler() {
    /*Acknowledge the PLT interrupt by loading the timer with a new value.*/
    setTIMER(TIMESLICE);

    /* Copy the processor state at the time of the exception (located at the start of the BIOS Data Page [Section ??-pops]) into the Current Pro- cess’s pcb (p_s). */
    state_t *exc_state = (state_t*) BIOSDATAPAGE;
    current_process->p_s = *exc_state;

    /* Place the Current Process on the Ready Queue; transitioning the Current Process from the “running” state to the “ready” state. */
    insertProcQ(&readyQ, current_process);

    /*Call the scheduler*/
    scheduling();
}

//3.6.3
void IT_interrupt_handler(){
    /*Acknowledge the interrupt by loading the Interval Timer with a new value: 100 milliseconds.*/
    LDIT(PSECOND);

    /*Unblock ALL pcbs blocked on the Pseudo-clock semaphore. Hence, the semantics of this semaphore are a bit different than traditional synchronization semaphores*/
    V_all();

    /*Return control to the Current Process: Perform a LDST on the saved exception state*/
    state_t *exc_state = (state_t*)BIOSDATAPAGE;
    LDST(exc_state);
}

/* ritorna la linea del device il cui interrupt è attivo */
int Get_interrupt_device(int IntLineNo)
{
    /* Calculate the address for this device’s device register */
    unsigned int interrupt_dev_bit_map = CDEV_BITMAP_ADDR(IntLineNo); 
    /*+indirizzo diverso in base al tipo di device */

    unsigned int int_linee[8];
    for (int i=0; i<8; i++) {
        unsigned mask = ((1<<1)-1)<<i;
        int_linee[i] = mask & interrupt_dev_bit_map;
    }
    
    /* int_linee[i] indica se la linea i-esima è attiva */
    int linea=0;
    while(linea<8) {
        if(int_linee[linea]!=0) { 
            break;
        }
        linea++;
    }
    
return linea;
}

//3.6.1     
void general_interrupt_handler(int IntLineNo)
{   /* vedere arch.h */
    int DevNo = Get_interrupt_device(IntLineNo);

    // Forse è possibile fare una funzione comune per tutti i device, passando IntLineNo per parametro
    
    /* Save off the status code from the device’s device register. */
    /*Uso la macro per trovare l'inidirzzo di base del device con la linea di interrupt e il numero di device*/
    dtpreg_t *dev_addr = (dtpreg_t*) DEV_REG_ADDR(IntLineNo,DevNo);
    /* Copia del device register*/
    dtpreg_t dev_reg;
    dev_reg.status = dev_addr->status;

    /* Acknowledge the outstanding interrupt. This is accomplished by writ-
        ing the acknowledge command code in the interrupting device’s device
        register. Alternatively, writing a new command in the interrupting
        device’s device register will also acknowledge the interrupt.*/
    dev_addr->command = ACK;

    /* Perform a V operation on the Nucleus maintained semaphore associ-
        ated with this (sub)device. This operation should unblock the process
        (pcb) which initiated this I/O operation and then requested to wait for
        its completion via a SYS5 operation.*/

    pcb_t *blocked_process = NULL;
    switch(IntLineNo){
        case DISKINT:
            blocked_process = headBlocked(&sem_disk[DevNo]);
            SYS_Verhogen(&sem_disk[DevNo]);
        case FLASHINT:
            blocked_process = headBlocked(&sem_tape[DevNo]);
            SYS_Verhogen(&sem_tape[DevNo]);
        case NETWINT:
            blocked_process = headBlocked(&sem_network[DevNo]);
            SYS_Verhogen(&sem_network[DevNo]);
        case PRNTINT:
            blocked_process = headBlocked(&sem_printer[DevNo]);
            SYS_Verhogen(&sem_printer[DevNo]);
        case TERMINT:
            blocked_process = headBlocked(&sem_terminal[DevNo]);
            SYS_Verhogen(&sem_terminal[DevNo]);
    }

    /* Place the stored off status code in the newly unblocked pcb’s v0 register.*/
    blocked_process->p_s.reg_v0=dev_reg.status;
    /* Insert the newly unblocked pcb on the Ready Queue, transitioning this
        process from the “blocked” state to the “ready” state*/

    /* Return control to the Current Process: Perform a LDST on the saved
        exception state (located at the start of the BIOS Data Page */
    state_t *prev_state = (state_t*)BIOSDATAPAGE;
    LDST(prev_state);
}

void terminal_interrupt_handler(){
    int DevNo = Get_interrupt_device(TERMINT);
   
    /* Save off the status code from the device’s device register. */
    /*Uso la macro per trovare l'inidirzzo di base del device con la linea di interrupt e il numero di device*/
    termreg_t *dev_addr= (termreg_t*) DEV_REG_ADDR(TERMINT,DevNo);
    /* Copia del device register*/
    termreg_t dev_reg;
    dev_reg.recv_status = dev_addr->recv_status;
    dev_reg.transm_status = dev_addr->transm_status;

    /* Acknowledge the outstanding interrupt. This is accomplished by writ-
        ing the acknowledge command code in the interrupting device’s device
        register. Alternatively, writing a new command in the interrupting
        device’s device register will also acknowledge the interrupt.*/
    /* Lo status code OKCHARTRANS (5) significa che il terminale ha generato l'interrupt per trasmettere, 
        altrimenti l'interrupt è stato generato per ricevere*/
    /* write utilizzato per salvare quale delle due operazioni stiamo eseguendo*/
    bool write = false;
    if(dev_reg.transm_status == OKCHARTRANS){
        dev_addr->transm_command = ACK;
        write = true;
    } else{
        dev_addr->recv_command = ACK;
        /*Aumenta DevNo per accedere poi ai campi di sem_interval
            Primi 8 in trasmissione ultimi 8 in ricezione*/
        DevNo += 8;
    }

    /* Perform a V operation on the Nucleus maintained semaphore associ-
        ated with this (sub)device. This operation should unblock the process
        (pcb) which initiated this I/O operation and then requested to wait for
        its completion via a SYS5 operation.*/

    pcb_t *blocked_process = headBlocked(&sem_terminal[DevNo]);
    SYS_Verhogen(blocked_process->p_semAdd);

    /* Place the stored off status code in the newly unblocked pcb’s v0 register.*/
    blocked_process->p_s.reg_v0 = write ? dev_reg.transm_status : dev_reg.recv_status;
    /* Insert the newly unblocked pcb on the Ready Queue, transitioning this
        process from the “blocked” state to the “ready” state*/
    insertProcQ(&readyQ, blocked_process);
    /* Return control to the Current Process: Perform a LDST on the saved
        exception state (located at the start of the BIOS Data Page */
    LDST(bios_State);
}

void V_all(){
    if(sem_interval_timer == 1) {
         /* chiamata allo scheduler*/
        scheduling();
    } else {
        while(headBlocked(&sem_interval_timer)!=NULL){
            pcb_t* wakedProc = removeBlocked(&sem_interval_timer);
            insertProcQ(&readyQ, wakedProc); 
        }
        sem_interval_timer = 1;
    }
}

#endif