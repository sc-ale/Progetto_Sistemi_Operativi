#ifndef INTERRUPT_C 
#define INTERRUPT_C

#include "interrupt.h"

/* serve per copiare, ad esempio, strutture dati */
void *memcpy(void *dest, const void *src, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        ((char*)dest)[i] = ((char*)src)[i];
    }
    return dest;
} 

/* Restituisce la linea con interrupt in attesa con massima priorità. 
(Se nessuna linea è attiva ritorna 8 ma assumiamo che quando venga
 chiamata ci sia almeno una linea attiva) */
int Get_Interrupt_Line () {
    unsigned int interrupt_pending = bios_State->cause & CAUSE_IP_MASK;
    /* Maschera i bit lasciando attivi quelli da 8 a 15 del cause register */
    unsigned int linea = 0;
    unsigned int intpeg_linee[8];
    /* Ignora l'8 bit del cause register */
    for (int i=1; i<8; i++) {
        unsigned mask = ((1<<1)-1)<<(i+8);
        intpeg_linee[i] = mask & interrupt_pending;
        /* intpeg_linee[i] indica se la linea i-esima è attiva */
        if(intpeg_linee[i]!=0) {
            linea = i;
            break;
        }
    }
    return linea;
}

/*
The interrupt exception handler’s first step is to determine which device
 or timer with an outstanding interrupt is the highest priority.
 Depending on the device, the interrupt exception handler will 
 perform a number of tasks.*/
void interrupt_handler() {
    /* Usiamo was_waiting per memorizzare se e' presente un processo a cui tornare il controllo */
    was_waiting = is_waiting;
    is_waiting = false;

    int line = Get_Interrupt_Line();
    
    switch (line) {
        /* interrupt processor Local Timer */
        case PLTINT:
            PLT_interrupt_handler();
            break;
        
        /* interrupt Interval Timer */
        case ITINT:
            IT_interrupt_handler();
            break;

        /* All devices except terminal */
        case DISKINT ... PRNTINT:
            general_interrupt_handler(line); 
            break;
        
        /* Terminal devices*/
        case TERMINT:
            terminal_interrupt_handler();
            break;

        default:
            break;
    }
   
    /* Verifica se c'e' un processo a cui tornare il controllo, altrimenti chiama lo scheduler */
    if (was_waiting) {
        scheduling();
    } else {
        LDST(bios_State);
    }

}

//3.6.2
void PLT_interrupt_handler() {
    /* ACK & LOAD del PLT */
    setTIMER(TIMESLICE);

    current_process->p_s = *bios_State;
    insertProcQ(&readyQ, current_process);

    scheduling();
}

//3.6.3
void IT_interrupt_handler() {
    /* ACK & LOAD dell'IT */
    LDIT(PSECOND);

    /* Sblocca tutti i processi bloccati sul semaforo dell'interval timer */
    while (headBlocked(&sem_interval_timer)!=NULL) {
        insertProcQ(&readyQ, removeBlocked(&sem_interval_timer)); 
        soft_block_count--;
    }
}

/* ritorna la linea del device il cui interrupt è attivo */
int Get_interrupt_device(int device_type) {
    /* Calcola l'indirizzo specifico del tipo di device */
    unsigned int *interrupt_dev_bit_map = (unsigned int*)CDEV_BITMAP_ADDR(device_type);

    unsigned int int_linee[8];
    int linea=0;

    for (int i=0; i<8; i++) {
        unsigned mask = ((1<<1)-1)<<i;
        int_linee[i] = mask & *interrupt_dev_bit_map;
        if(int_linee[linea]!=0) { 
            linea = i;
            break;
        }
    }
    return linea;
}

//3.6.1     
void general_interrupt_handler(int device_type) {   /* vedere arch.h */
    int DevNo = Get_interrupt_device(device_type);

    // Forse è possibile fare una funzione comune per tutti i device, passando device_type per parametro
    
    /* Save off the status code from the device’s device register. */
    /*Uso la macro per trovare l'inidirzzo di base del device con la linea di interrupt e il numero di device*/
    dtpreg_t *dev_addr = (dtpreg_t*) DEV_REG_ADDR(device_type,DevNo);
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
    switch(device_type){
        case DISKINT:
            blocked_process = headBlocked(&sem_disk[DevNo]);
            SYS_Verhogen(&sem_disk[DevNo]);
            break;
        case FLASHINT:
            blocked_process = headBlocked(&sem_tape[DevNo]);
            SYS_Verhogen(&sem_tape[DevNo]);
            break;
        case NETWINT:
            blocked_process = headBlocked(&sem_network[DevNo]);
            SYS_Verhogen(&sem_network[DevNo]);
            break;
        case PRNTINT:
            blocked_process = headBlocked(&sem_printer[DevNo]);
            SYS_Verhogen(&sem_printer[DevNo]);
            break;
        case TERMINT:
            blocked_process = headBlocked(&sem_terminal[DevNo]);
            SYS_Verhogen(&sem_terminal[DevNo]);
            break;
    }

    /* Place the stored off status code in the newly unblocked pcb’s v0 register.*/
    blocked_process->p_s.reg_v0=dev_reg.status;
    /* Insert the newly unblocked pcb on the Ready Queue, transitioning this
        process from the “blocked” state to the “ready” state*/

    /* Return control to the Current Process: Perform a LDST on the saved
        exception state (located at the start of the BIOS Data Page */
    LDST(bios_State);
}

void terminal_interrupt_handler(){
    int DevNo = Get_interrupt_device(TERMINT);
   
    /* Save off the status code from the device’s device register. */
    /*Uso la macro per trovare l'inidirzzo di base del device con la linea di interrupt e il numero di device*/
    termreg_t *dev_addr = (termreg_t*) DEV_REG_ADDR(TERMINT,DevNo);
    /* Acknowledge the outstanding interrupt. This is accomplished by writ-
        ing the acknowledge command code in the interrupting device’s device
        register. Alternatively, writing a new command in the interrupting
        device’s device register will also acknowledge the interrupt.*/
    /* Lo status code OKCHARTRANS (5) significa che il terminale ha generato l'interrupt per trasmettere, 
        altrimenti l'interrupt è stato generato per ricevere*/
    /* write utilizzato per salvare quale delle due operazioni stiamo eseguendo*/
    unsigned int device_response;
    if((dev_addr->transm_status & BYTE1MASK) == OKCHARTRANS){
        device_response = dev_addr->transm_status;
        dev_addr->transm_command = ACK;
        /*Aumenta DevNo per accedere poi ai campi di sem_interval
        Primi 8 in ricezione ultimi 8 in trasmissione*/
        DevNo += 8;
    } else if((dev_addr->recv_status & BYTE1MASK) == OKCHARTRANS){
        device_response = dev_addr->recv_status;
        dev_addr->recv_command = ACK;
    }

    /* Perform a V operation on the Nucleus maintained semaphore associ-
        ated with this (sub)device. This operation should unblock the process
        (pcb) which initiated this I/O operation and then requested to wait for
        its completion via a SYS5 operation.*/
    pcb_t *blocked_process = headBlocked(&sem_terminal[DevNo]);
    SYS_Verhogen(blocked_process->p_semAdd);
    soft_block_count--;

    /* Place the stored off status code in the newly unblocked pcb’s v0 register.*/
    //blocked_process->p_s.reg_v0 = write ? dev_addr->transm_status : dev_addr->recv_status;
    blocked_process->p_s.reg_v0 = 0;
    *blocked_process->IOvalues = device_response;
    /* Insert the newly unblocked pcb on the Ready Queue, transitioning this
        process from the “blocked” state to the “ready” state*/
    /* InsertProcQ lo fa già nella Verhogen */
    /* Return control to the Current Process: Perform a LDST on the saved
        exception state (located at the start of the BIOS Data Page */
}
#endif