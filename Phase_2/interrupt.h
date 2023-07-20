#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <umps3/umps/arch.h>
#include "scheduler.h"

extern state_t* bios_State;

extern int* deviceType2Sem(int);
extern void SYS_verhogen(int*);

/* Restituisce la linea con interrupt in attesa con massima priorità */
int get_interrupt_line ();

/* Seleziona quale gestore chiamare in base alla linea di interrupt */
void interrupt_handler();

/* -- Gestori interrupt local timer e interval timer -- */
void PLT_interrupt_handler();
void IT_interrupt_handler();

/* Ritorna la linea del device il cui interrupt è attivo */
int get_interrupt_device(int ); 

/* -- Gestori dei device -- */  
void general_interrupt_handler(int );
void terminal_interrupt_handler();

#endif