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
        /* se inseriamo p in pcbFree_h non dovremmo staccarlo da dove era collegato ? */
        list_add(&p->p_list, &pcbFree_h);
}

/* TEMPORANEO: Forse bisogna controllare l'inizializzazione di cpu_time e state_t*/
/* Restituisce NULL se la pcbFree_h è vuota. Altrimenti rimuove un elemento dalla pcbFree, 
inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso */
pcb_t *allocPcb()
{
        if(list_empty(&pcbFree_h)) {
                return NULL;
        }

        else {
                struct pcb_t *p=list_first_entry(&pcbFree_h,struct pcb_t, p_list);
                list_del(&p->p_list);
                p->p_parent=NULL;
                INIT_LIST_HEAD(&p->p_list);
                INIT_LIST_HEAD(&p->p_child);
                INIT_LIST_HEAD(&p->p_sib);
                p->p_semAdd=NULL; /* FIX ME*/
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
        return list_empty(&p->p_child);
}

/* Inserisce il PCB puntato da p come figlio del PCB puntato da prnt */
void insertChild(pcb_t *prnt, pcb_t *p)
{       
        p->p_parent=prnt;
        if(emptyChild(prnt)) { 
                /* prnt non ha figli */
                INIT_LIST_HEAD(&p->p_child);
                prnt->p_child.next=&p->p_child;
        }
        
        else { 
                /* se prnt ha dei figli dobbiamo mettere p come fratello dei figli */
                pcb_t *figlioPrnt = list_first_entry(&prnt->p_child, pcb_t, p_child);
                list_add_tail(&p->p_sib, &figlioPrnt->p_sib);
        }
}

/* Rimuove il PCB puntato da p dalla lista dei figli del padre. 
 Se il PCB puntato da p non ha un padre, restituisce NULL, altrimenti restituisce l’elemento rimosso (cioè p). 
 A differenza della removeChild, p può trovarsi in una posizione arbitraria 
 (ossia non è necessariamente il primo figlio del padre, 
 in questo caso p è il figlio che va rimosso, invece nella removeChild p è il padre) */
pcb_t *outChild(pcb_t *p)
{
        if(p->p_parent!=NULL) {
                pcb_t *padreP = p->p_parent;
                if(padreP->p_child.next==p->p_child.prev){
                        /* p è il primo figlio*/
                        if(list_empty(&p->p_sib)) INIT_LIST_HEAD(&padreP->p_child);
                        
                        else { 
                                /* p ha almeno un fratello */
                                pcb_t *fratello = list_first_entry(&p->p_sib, pcb_t, p_sib);
                                padreP->p_child.next=fratello->p_child.prev;
                        }
                }

                list_del_init(&p->p_sib);
                return p;
        }
        
        else return NULL;
}

/* Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL */
pcb_t *removeChild(pcb_t *p)
{  
        pcb_t *primoFiglio = list_first_entry(&p->p_child, pcb_t, p_child);        
        return outChild(primoFiglio);
}

/* TEMPORANEO: TUTTE LE FUNZIONI SOPRA FUNZIONANO */