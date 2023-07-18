#ifndef INTERRUPT_C 
#define INTERRUPT_C

#include "interrupt.h"


int get_interrupt_line () {
    unsigned int interrupt_pending = bios_State->cause & CAUSE_IP_MASK;
    /* Maschera i bit lasciando attivi quelli da 8 a 15 del cause register */
    unsigned int linea = 0;
    unsigned int intpeg_linee[8];
    /* Ignora l'8 bit del cause register */
    for (int i=1; i<8; i++) {
        unsigned mask = 1<<(i+8);
        intpeg_linee[i] = mask & interrupt_pending;
        /* intpeg_linee[i] indica se la linea i-esima è attiva */
        if(intpeg_linee[i]!=0) {
            linea = i;
            break;
        }
    } 
    /* Se nessuna linea è attiva ritorna 8 ma assumiamo che 
     quando chiamata ci sia almeno una linea attiva */
    return linea;
}


void interrupt_handler() {
    
    int line = get_interrupt_line();
    
    switch (line) {
        /* Interrupt processor Local Timer */
        case PLTINT:
            PLT_interrupt_handler();
            break;
        
        /* Interrupt Interval Timer */
        case ITINT:
            IT_interrupt_handler();
            break;

        /* All devices except terminal */
        case DISKINT ... PRNTINT:
            general_interrupt_handler(line); 
            break;
        
        /* Terminal devices */
        case TERMINT:
            terminal_interrupt_handler();
            break;

        default:
            /* Non ci sono linee di interrupt attive */
            break;
    }
   
    /* Verifica se c'e' un processo a cui tornare il controllo, altrimenti chiama lo scheduler */
    if (is_waiting) {
        is_waiting = false;
        scheduling();
    } else {
        LDST(bios_State);
    }
}


void PLT_interrupt_handler() {
    /* ACK & LOAD del PLT */
    setTIMER(TIMESLICE);

    current_process->p_s = *bios_State;
    insertProcQ(&readyQ, current_process);

    scheduling();
}


void IT_interrupt_handler() {
    /* ACK & LOAD dell'IT */
    LDIT(PSECOND);

    /* Sblocca tutti i processi bloccati sul semaforo dell'interval timer */
    while (headBlocked(&sem_interval_timer)!=NULL) {
        insertProcQ(&readyQ, removeBlocked(&sem_interval_timer)); 
        soft_block_count--;
    }
}


int get_interrupt_device(int device_type) {
    /* Calcola l'indirizzo specifico del tipo di device */
    unsigned int *interrupt_dev_bit_map = (unsigned int*)CDEV_BITMAP_ADDR(device_type);

    unsigned int int_linee[8];
    int linea=0;

    for (int i=0; i<8; i++) {
        unsigned mask = 1<<i;
        int_linee[i] = mask & *interrupt_dev_bit_map;
        if(int_linee[linea]!=0) { 
            linea = i;
            break;
        }
    }
    return linea;
}


void general_interrupt_handler(int device_type) {
    /* Calcola l'indirizzo del device register utilizzando la linea di interrupt e il numero di device */
    int devNo = get_interrupt_device(device_type);
    dtpreg_t *dev_addr = (dtpreg_t*) DEV_REG_ADDR(device_type,devNo);
    dtpreg_t dev_reg;
    dev_reg.status = dev_addr->status;

    /* Acknowledgment dell'interrupt */
    dev_addr->command = ACK;

    /* Sblocca il processo che ha iniziato l'operazione di I/O */
    pcb_t *blocked_process = NULL;
    /* Si sottrae 3 a device_type siccome questo assume i valori da 3 a 7 */
    int* sem2use = deviceType2Sem(device_type-3);
    blocked_process = headBlocked(&sem2use[devNo]);
    SYS_verhogen(&sem2use[devNo]);
    soft_block_count--;

    /* Salva lo stato del device nel pcb sbloccato */
    blocked_process->p_s.reg_v0 = 0;
    *blocked_process->IOvalues = dev_reg.status;
}


void terminal_interrupt_handler(){
    /* Calcola l'indirizzo del device register utilizzando la linea di interrupt e il numero di device */
    int devNo = get_interrupt_device(TERMINT);
    termreg_t *dev_addr = (termreg_t*) DEV_REG_ADDR(TERMINT,devNo);

    /* Verifca quale opearazione ha eseguito controllando lo status in trasmissione o recezione */
    unsigned int device_response;
    if((dev_addr->transm_status & BYTE1MASK) == OKCHARTRANS){
        /* Salva lo stato del device in trasmissione e da l'acknowledgment */
        device_response = dev_addr->transm_status;
        dev_addr->transm_command = ACK;
        
        /* Aumenta devNo per accedere al semaforo di devNo riservato alla trasmissione */
        devNo += 8;
    } else if((dev_addr->recv_status & BYTE1MASK) == OKCHARTRANS){
        /* Salva lo stato del device in ricezione e da l'acknowledgment */
        device_response = dev_addr->recv_status;
        dev_addr->recv_command = ACK;
    }

    /* Sblocca il processo che ha iniziato l'operazione di I/O */
    pcb_t *blocked_process = headBlocked(&sem_terminal[devNo]);
    SYS_verhogen(blocked_process->p_semAdd);
    soft_block_count--;

     /* Salva lo stato del device nel pcb sbloccato */
    blocked_process->p_s.reg_v0 = 0;
    *blocked_process->IOvalues = device_response;
}

#endif