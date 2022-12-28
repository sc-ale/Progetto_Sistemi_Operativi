#include <stdio.h>
#include "pandos_types.h"

pcb_t pcbFree_table[MAXPROC];

struct pcbFree_h{
    pcb_t p;
    struct list_head pcbFree;
} pcbFree_h;

/* Inserisce, dopo la testa, il pcb puntato in pcbFree_h -- funzione 2 */
extern void FreePcb(pcb_t *p)
{
    p->p_s='waiting'; /*processo libero o inutilizzato, 
    non trovo i possibili valori di state_t */
    list_add(p->p_list, pcbFree_h->pcbFree);
}

/* Inserisce l’elemento puntato da p nella coda dei processi puntata da head*/
extern insertProcQ( struct list_head *head, pcb_t*p)
{
    list_add_tail(p->p_list, head);  
}
/* Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti */
extern int emptyChild(pcb_t *p)
{   
    int child = (p->p_child==NULL)? 1:0;
    return child;
}