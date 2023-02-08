#include <stdio.h>
#include "pandos_types.h"
#include "list.h"
#include "types.h"

struct list_head *pcbFree_h;
pcb_t pcbFree_table[MAXPROC];

void initPcbs()
{
        INIT_LIST_HEAD(pcbFree_h);
        for(int i=0; i<MAXPROC; i++){
            list_add_tail(&pcbFree_table[i].p_list,pcbFree_h);
        }
}

void freePcb(pcb_t *p)
{
        list_add(&p->p_list,pcbFree_h);
}

pcb_t *allocPcb()
{
    if(list_empty(pcbFree_h)) {
            return NULL;
    }

    else {
        pcb_t *p=pcbFree_h->next;
        list_del(pcbFree_h->next);
        p->p_child,p->p_parent,p->p_sib=NULL;
        p->p_semAdd=0;
        return p;
    }
}