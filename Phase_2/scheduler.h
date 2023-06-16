#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include "initial.h"

extern int process_count; 
extern int soft_block_count; 
extern struct list_head readyQ;
extern pcb_t* current_process; 
extern int sem_processor_local_timer;
extern int sem_interval_timer;
extern int sem_disk[8];
extern int sem_tape[8];
extern int sem_network[8];
extern int sem_printer[8];
extern int sem_terminal[16];


void scheduling();

#endif 