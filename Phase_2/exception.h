#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <umps3/umps/arch.h>
#include "interrupt.h"

#define UPDATE_PC bios_State->pc_epc += WORDLEN;
#define SAVESTATE current_process->p_s = *bios_State
#define UPDATE_BIOSSTATE_REGV0(T) bios_State->reg_v0 = (unsigned int)T;  

pcb_t pcbFree_table[MAXPROC];
state_t *bios_State;

/* come process id usiamo un intero che aumenta
    e basta (no caso reincarazione)*/
int pid_start = 1;

void foobar();

void updateCPUtime();

void passup_ordie(int );

void syscall_handler();

static void SYS_create_process(state_t *, support_t *, nsd_t *);

static void SYS_terminate_process(int );

void terminate_family(int);

void terminate_family2(int);

bool is_sem_device_or_int(int*);

pcb_t* getProcByPid(int);

void kill_process(pcb_t *);

static void SYS_Passeren(int *);

void SYS_Verhogen(int *);

static void SYS_Doio(int *, int *);

void OPERAZIONICOMUNIDOIO123();

static void SYS_Get_CPU_Time();

static void SYS_Clockwait();

static void SYS_Get_Support_Data();

static void SYS_Get_Process_Id(int );

static void SYS_Get_Children(int *, int );

int Check_Kernel_mode();


#endif