#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <umps3/umps/arch.h>
#include "scheduler.h"

#define PLTINT  1
#define ITINT   2
extern state_t* bios_State;

/* Variabile locale usata per capire se c'e' un processo a cui ritornare il controllo*/
extern bool was_waiting;

extern int* deviceType2Sem(int);
extern void SYS_verhogen(int*);

/* Serve per copiare, ad esempio, strutture dati */
void *memcpy(void *, const void *, unsigned int );

/* Restituisce la linea con interrupt in attesa con massima priorità */
int get_interrupt_line ();

/* The interrupt exception handler’s first step is to determine which device
 or timer with an outstanding interrupt is the highest priority.
 Depending on the device, the interrupt exception handler will 
 perform a number of tasks.*/
void interrupt_handler();

//3.6.2
void PLT_interrupt_handler();
//3.6.3
void IT_interrupt_handler();

/* Ritorna la linea del device il cui interrupt è attivo */
int get_interrupt_device(int ); 

//3.6.1     
void general_interrupt_handler(int );

void terminal_interrupt_handler();

#endif