#include <stdio.h>
#include "pandos_types.h"
#include "list.h"
#include "types.h"

LIST_HEAD(pcbFree_h);
pcb_t pcbFree_table[MAXPROC];

/* Inizializzazione di pcbFree_h inserendo gli elementi in pcbFree_table*/
void initPcbs()
{       
        for(int i=0; i<MAXPROC; i++){
            list_add(&pcbFree_table[i].p_list, &pcbFree_h);
        }
}

/* Inserisce il PCB puntato da p in pcbFree_h*/
void freePcb(pcb_t *p)
{
        list_add(&p->p_list, &pcbFree_h);
}

/* Forse bisogna controllare l'inizializzazione di cpu_time e state_t*/
pcb_t *allocPcb()
{
        if(list_empty( &pcbFree_h)) {
                return NULL;
        }

        else {
            struct pcb_t *p = NULL;
            p=list_first_entry(&pcbFree_h,struct pcb_t, p_list);
            list_del(&p->p_list);
            p->p_parent=NULL;
            p->p_child.next=NULL;
            p->p_child.prev=NULL;
            p->p_sib.next=NULL;
            p->p_sib.prev=NULL;
            p->p_semAdd=NULL;
            return p;
        }
}

/* crea una lista di pcb inizializzandola come vuota*/
void mkEmptyProcQ(struct list_head *head)
{
        INIT_LIST_HEAD(head);
}

/* Restrituisce true se la lista puntata da head è vuota, false altrimenti*/
int emptyProcQ(struct list_head *head)
{
        return list_empty(head);
}

/* Inserisce l’elemento puntato da p nella coda dei processi puntata da head */
void insertProcQ(struct list_head *head, pcb_t* p)
{
        list_add_tail(&p->p_list, head);
}

/* Restituisce l’elemento di testa di head senza rimuoverlo.
Ritorna NULL se la coda è vuota*/
pcb_t *headProcQ(struct list_head *head)
{
        pcb_t *p = (!emptyProcQ(head)) ? list_first_entry(head,struct pcb_t, p_list) : NULL;
        return p;
}

/* Rimuove il primo elemento dalla coda dei processi puntata da head. 
 Ritorna NULL se la coda è vuota.
 Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.*/
pcb_t *removeProcQ(struct list_head *head)
{
        pcb_t *p = (!emptyProcQ(head)) ? list_first_entry(head,struct pcb_t, p_list) : NULL;
        if (!emptyProcQ(head)) list_del(&p->p_list);
        return p;
}

/* Rimuove il PCB puntato da p dalla coda dei processi puntata da head.
 Se p non è presente nella coda, restituisce NULL. 
 (NOTA: p può trovarsi in una posizione arbitraria della coda).*/
pcb_t *outProcQ(struct list_head* head, pcb_t *p)
{
    struct list_head *corrente1, *corrente2 = NULL;
    struct pcb_t *temp, *trovato = NULL;
    list_for_each_safe(corrente1, corrente2, head) 
    {
            temp = list_entry(corrente1, struct pcb_t, p_list);
            if (temp==p) { 
                trovato = temp;
                list_del(&temp->p_list);
            }   
    } 
   return trovato;
}


/* Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti */
int emptyChild(pcb_t *p)
{
        return p->p_child==NULL;
}

/* Inserisce il PCB puntato da p come figlio del PCB puntato da prnt */
void insertChild(pcb_t *prnt, pcb_t *p)
{
        prnt->p_child=p->p_list;
}

/* Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL */
pcb_t* removeChild(pcb_t *p)
{
        
}
