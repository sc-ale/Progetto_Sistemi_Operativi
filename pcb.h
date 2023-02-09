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
        INIT_LIST_HEAD(&head);
}

/* Restrituisce true se la lista puntata da head è vuota, false altrimenti*/
int emptyProcQ(struct list_head *head)
{
        return list_empty(&head);
}

/* Inserisce l’elemento puntato da p nella coda dei processi puntata da head */
void insertProcQ(struct list_head *head, pcb_t* p)
{
        list_add(&p->p_list, &head);
}

/* Restituisce l’elemento di testa di head senza rimuoverlo.
Ritorna NULL se la coda è vuota*/
pcb_t headProcQ(struct list_head *head)
{
        if (!emptyProcQ(&head)){
            return list_first_entry(&head, struct pcb_t, p_list);
        } 

        else {
            return NULL;
        }
}

/* Rimuove il primo elemento dalla coda dei processi puntata da head. 
 Ritorna NULL se la coda è vuota.
 Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.*/
pcb_t *removeProcQ(struct list_head *head)
{
    if (!emptyProcQ(&head)){    
            struct pcb_t *p = NULL;
            p = list_first_entry(&head,struct pcb_t, p_list);
            list_del(&p->p_list);
            return p;
        } 

        else {
            return NULL;
        }
}

/* Rimuove il PCB puntato da p dalla coda dei processi puntata da head.
 Se p non è presente nella coda, restituisce NULL. 
 (NOTA: p può trovarsi in una posizione arbitraria della coda).*/
pcb_t *outProcQ(struct list_head* head, pcb_t *p)
{
    int trovato = 0;
    struct list_head *elCorrente = NULL;
    struct pcb_t* temp = NULL;
    list_for_each(elCorrente, &head)
    {
            temp = list_entry(elCorrente, struct pcb_t, p_list);
            if (temp==p) trovato = 1;
    } 

    if(trovato) {
            list_del(&p);
            return p;
    }

    else {
            return NULL;
    }
}