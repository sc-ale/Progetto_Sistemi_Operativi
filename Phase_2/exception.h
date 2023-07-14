#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <umps3/umps/arch.h>
#include "interrupt.h"

/* Incrementa il program counter */
#define UPDATE_PC bios_State->pc_epc += WORDLEN;
#define SAVESTATE current_process->p_s = *bios_State

/* Salva T nel registro reg_v0 */
#define UPDATE_BIOSSTATE_REGV0(T) bios_State->reg_v0 = (unsigned int)T;  
/* Verifica se T è un semaforo di un device o dell'interval timer */
#define IS_SEM_DEVICE_OR_INT(T) (T == &sem_interval_timer || T == sem_disk || T == sem_network || T == sem_printer || T == sem_tape || T == sem_terminal)

state_t *bios_State;
bool  was_waiting;

/* Come process id usiamo un intero che aumenta */
int pid_start = 1;

/* CONTROLLARE LA SEZIONE 3.5.12 */
void exception_handler();

/* Aggiorna il p_time del processo corrente */
void updateCPUtime();

// DA RIGUARDARE 3.7*/
void passup_ordie(int );

/* Gestisce quale sysstem call chiamare */
void syscall_handler();

/** 
 * Crea un nuovo processo come figlio del chiamante e lo inizializza con la struttura di
 * supporto e il namespace dati. Se quest'ultimo non è specificato eredita il ns del padre 
 **/
static void SYS_create_process(state_t *, support_t *, nsd_t *);

/* Termina il processo con identificativo pid e tutti suoi figli */
static void SYS_terminate_process(int );

/* Funzioni ausiliare alla SYS_terminate_process */
void terminate_family(int);
void kill_process(pcb_t *);
pcb_t* getProcByPid(int);

/* Operazione di Passeren e Verhogen sul semaforo passato */
static void SYS_passeren(int *);
void SYS_verhogen(int *);

/* Effettua un'operazione di I/O */
static void SYS_doio(int *, int *);

/* Funzione ausiliaria alla SYS doio */
void general_doio(int *, int *, int, int);

/* Restiuisce il puntatore al semaforo relativo al tipo di device */
int* deviceType2Sem(int);

/* Restituisce il tempo di utilizzo del processore del processo in esecuzione */
static void SYS_get_CPU_time();

/**
 * Equivalente a una Passeren sul semaforo dell’Interval Timer.
 * Blocca il processo invocante fino al prossimo tick del dispositivo.
 **/
static void SYS_clockwait();

/* Restituisce un puntatore alla struttura di supporto del processo corrente */
static void SYS_get_support_data();

/**
 * Restituisce l’identificatore del processo invocante se parent == 0,
 * quello del genitore se quest'ultimo e il figlio sono nello stesso namespace.
 **/
static void SYS_get_process_id(int );

/**
 * Ritorna il numero di figli che appartengono allo stesso namespace del chiamante
 * e salva i rispettivi pid nell'array children di dimensione size 
 **/
static void SYS_get_children(int *, int );

/* Determina se il processo corrente stava eseguento in kernel o user mode */
bool Check_Kernel_mode();

#endif