#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <umps3/umps/arch.h>
#include "interrupt.h"

#define UPDATE_PC bios_State->pc_epc += WORDLEN;
#define SAVESTATE current_process->p_s = *bios_State
#define UPDATE_BIOSSTATE_REGV0(T) bios_State->reg_v0 = T;  

pcb_t pcbFree_table[MAXPROC];
state_t *bios_State;

/* come process id usiamo un intero che aumenta
    e basta (no caso reincarazione)*/
int pid_start = 0;

unsigned int aaaTest_variable = 0;
unsigned int* aaaTest_Supremo = NULL;
unsigned int aaa_val_di_Test_variabile = 0;

void foobar();

void updateCPUtime();

void passup_ordie(int );

void syscall_handler();

void update_PC_SYS_non_bloccanti();

void update_PC_SYS_bloccanti();

void SYS_create_process(state_t *, support_t *, nsd_t *);

void SYS_terminate_process(int );

void terminate_family(int);

pcb_t* getProcByPid(int);

void kill_process(pcb_t *);

void SYS_Passeren(int *);

void SYS_Verhogen(int *);

void SYS_Doio(int *, int *);

void OPERAZIONICOMUNIDOIO123();

void SYS_Get_CPU_Time();

void SYS_Clockwait();

support_t* SYS_Get_Support_Data();

void SYS_Get_Process_Id(int );

void SYS_Get_Children(int *, int );

int Check_Kernel_mode();

void P_always(int *);


/* funzioni per test */
void aaaCASO_0_test(){}
void aaaCASO_1_test(){}
void aaaCASO_2_test(){}
void aaaCASO_3_test(){}
void aaaCASO_4_test(){}


#endif