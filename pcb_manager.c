#include <stdio.h>
#include "pandos_types.h"

pcb_t pcbFree_table[MAXPROC];

typedef struct pcbFree_h{
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

/* Ritorna Null se la lista di process liberi è vuota, altrimenti rimuove e ritorna un elemento inizializzandolo a NULL/0 -- funzione 3*/

pcb_t *allocPcb(){
    if(free_head == NULL){
        return NULL;
    } else{
        pcb_t *temp_pcb = free_head -> p;
        pcbFree_h *temp_struct = free_head;
        free_head = free_head -> pcbFree;
        delete(temp_struct);

        temp_pcb->p_list.next = NULL;
        temp_pcb->p_list.prev = NULL;
        temp_pcb->p_parent = NULL;
        temp_pcb->p_child.next = NULL;
        temp_pcb->p_child.prev = NULL;
        temp_pcb->p_sib.next = NULL;
        temp_pcb->p_sib.prev = NULL;
        temp_pcb->p_s = NULL; // p_s non definito
        temp_pcb->p_time = 0;
        temp_pcb->p_semAdd = NULL;
        temp_pcb->namespaces = NULL;

        return temp_pcb;
    }
}

/* Inserisce l’elemento puntato da p nella coda dei processi puntata da head -- funzione 6 */
extern insertProcQ( struct list_head *head, pcb_t*p)
{
    list_add_tail(p->p_list, head);  
}
/* Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti -- funzione 10*/
extern int emptyChild(pcb_t *p)
{   
    int child = (p->p_child==NULL)? 1:0;
    return child;
}

