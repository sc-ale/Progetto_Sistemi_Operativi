#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <umps3/umps/arch.h>

#include "scheduler.h"

extern state_t* bios_State;

extern void SYS_Verhogen(int*);

extern unsigned int aaaTest_variable;


void *memcpy(void *, const void *, unsigned int );

/* Restituisce la linea con interrupt in attesa con massima priorità. 
(Se nessuna linea è attiva ritorna 8 ma assumiamo che quando venga
 chiamata ci sia almeno una linea attiva) */
int Get_Interrupt_Line_Max_Prio ();

/* The interrupt exception handler’s first step is to determine which device
 or timer with an outstanding interrupt is the highest priority.
 Depending on the device, the interrupt exception handler will 
 perform a number of tasks.*/
void interrupt_handler();

void PLT_interrupt_handler();

void IT_interrupt_handler();

/* ritorna la linea del device il cui interrupt è attivo */
int Get_interrupt_device(int ); 

void general_interrupt_handler(int );

void terminal_interrupt_handler();




#endif