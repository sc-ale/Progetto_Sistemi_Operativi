#include "pcb.h"

/* variabili globali */
HIDDEN LIST_HEAD(pcbFree_h);
HIDDEN pcb_t pcbFree_table[MAXPROC];


void initPcbs()
{       
        for(int i=0; i<MAXPROC; i++){
                list_add(&pcbFree_table[i].p_list, &pcbFree_h);
        }
}


void freePcb(pcb_t *p)
{
        list_add(&p->p_list, &pcbFree_h);
}


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
                p->p_semAdd=NULL;
                return p;
        }
}


void mkEmptyProcQ(struct list_head *head)
{
        INIT_LIST_HEAD(head);
}


int emptyProcQ(struct list_head *head)
{
        return list_empty(head);
}


void insertProcQ(struct list_head *head, pcb_t* p)
{
        list_add_tail(&p->p_list, head);
}


pcb_t *headProcQ(struct list_head *head)
{
        pcb_t *p = (!emptyProcQ(head)) ? list_first_entry(head,struct pcb_t, p_list) : NULL;
        return p;
}


pcb_t *removeProcQ(struct list_head *head)
{
        pcb_t *p = (!emptyProcQ(head)) ? list_first_entry(head,struct pcb_t, p_list) : NULL;
        if (!emptyProcQ(head)) list_del(&p->p_list);
        return p;
}


pcb_t *outProcQ(struct list_head* head, pcb_t *p)
{
        /* utilizziamo corrente1 e corrente2 per iterare sulla lista head */
        struct list_head *corrente1, *corrente2 = NULL;

        /* utilizziamo temp per verificae se l'elemento corrente corrisponde a p 
        e in tal caso lo salviamo in trovato */
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


int emptyChild(pcb_t *p)
{
        return list_empty(&p->p_child);
}


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


pcb_t *outChild(pcb_t *p)
{
        if(p->p_parent!=NULL) {
                pcb_t *padreP = p->p_parent;
                if(padreP->p_child.next==p->p_child.prev){
                        /* p è il primo figlio */
                        if(list_empty(&p->p_sib)) INIT_LIST_HEAD(&padreP->p_child);
                        
                        else { 
                                /* p ha almeno un fratello */
                                pcb_t *fratello = list_first_entry(&p->p_sib, pcb_t, p_sib);
                                padreP->p_child.next=fratello->p_child.prev;
                        }
                }
                p->p_parent = NULL;
                list_del_init(&p->p_sib);
                return p;
        }
        
        else return NULL;
}


pcb_t *removeChild(pcb_t *p)
{  
        pcb_t *primoFiglio = list_first_entry(&p->p_child, pcb_t, p_child);        
        return outChild(primoFiglio);
}