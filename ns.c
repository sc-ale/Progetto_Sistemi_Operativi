#include "ns.h"


HIDDEN struct list_head type_nsFree_h[NS_TYPE_MAX];
HIDDEN struct list_head type_nsList_h[NS_TYPE_MAX];
HIDDEN nsd_t type_nsd[MAXPROC][NS_TYPE_MAX];

void initNamespaces()
{
    int i, j;

    //inizializza le liste 
    for(i=0;i<NS_TYPE_MAX;i++)
    {
        INIT_LIST_HEAD(&type_nsFree_h[i]);
        INIT_LIST_HEAD(&type_nsList_h[i]);
    }

    // aggiungo gli NSD alla free list
    for(i=0;i<MAXPROC;i++)
    {
        for(j=0;j<NS_TYPE_MAX;j++)
        {
            list_add(&type_nsd[i][j].n_link,&type_nsFree_h[j]);
        }
    }

}


nsd_t *getNamespace(pcb_t *p, int type)
{
    if(p==NULL || type<0 || type > NS_TYPE_LAST)
    {
        return NULL;
    }

    else return p->namespaces[type];
}

int addNamespace(pcb_t *p, nsd_t *ns)
{
    if(p==NULL || ns==NULL){
        return FALSE;
    }

    //associo il namespace al processo corrente
    p->namespaces[ns->n_type]=ns;
    //associo il namespace ai figli del processo 
    if(!list_empty(&p->p_child)){
        pcb_t *child = list_first_entry(&p->p_child, pcb_t, p_child);
        child->namespaces[ns->n_type] = ns;

        list_for_each_entry(child, &child->p_sib, p_sib){
            child->namespaces[ns->n_type] = ns;
        }
    }
    return TRUE;
}


nsd_t *allocNamespace(int type){
    if(type<0 || type>NS_TYPE_LAST){
        return NULL;
    }
    if(list_empty(&type_nsFree_h[type])){
        return NULL;
    }
    nsd_t *new_nsd = list_first_entry(type_nsFree_h, nsd_t, n_link);
    list_del_init(&new_nsd->n_link);
    list_add(&new_nsd->n_link, &type_nsList_h[type]);
    return new_nsd;
}



void freeNamespace(nsd_t *ns){
    list_del_init(&ns->n_link);
    list_add(&ns->n_link, &type_nsFree_h[ns->n_type]);
}