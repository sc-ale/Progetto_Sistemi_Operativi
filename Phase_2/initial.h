#ifndef INITIAL_H
#define INITIAL_H

#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <pandos_types.h>
#include <pandos_const.h>
#include "pcb.h"
#include "ash.h"
#include "ns.h"

/* -- Dichiarazioni di funzioni globali -- */
extern void uTLB_RefillHandler();
extern void test();
extern void scheduling();
extern void exception_handler();
extern void *memcpy(void *, const void *, unsigned int );

#endif 