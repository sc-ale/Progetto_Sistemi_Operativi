#include "ns.h"

/* variabili globali */
HIDDEN struct list_head type_nsFree_h[NS_TYPE_MAX];
HIDDEN struct list_head type_nsList_h[NS_TYPE_MAX];
HIDDEN nsd_t type_nsd[MAXPROC][NS_TYPE_MAX];


void initNamespaces()
{
    /* inizializza le liste */
    for(int i=0;i<NS_TYPE_MAX;i++)
    {
        INIT_LIST_HEAD(&type_nsFree_h[i]);
        INIT_LIST_HEAD(&type_nsList_h[i]);

        /* aggiungo gli NSD alla free list */
        for(int j=0;j<MAXPROC;j++) 
        {   
            type_nsd[i][j].n_type = i; 
            list_add(&type_nsd[i][j].n_link,&type_nsFree_h[i]);
        }
    }
}


nsd_t *getNamespace(pcb_t *p, int type)
{   
    /* verifica che il tipo assegnato sia valido */
    /*
    return (p==NULL || type<0 || type > NS_TYPE_LAST) ? NULL : p->namespaces[type];
    */
   for (int i=0; i<NS_TYPE_MAX; i++){
    	if (p->namespaces[i]->n_type==type){
    	    return p->namespaces[i];	
    	}
    }
    return NULL;
}

int addNamespace(pcb_t *p, nsd_t *ns)
{
    if(p==NULL || ns==NULL) {
        return FALSE;
    }

    /* associo il namespace al processo corrente */
    p->namespaces[ns->n_type]=ns;

    /* associo il namespace ai figli del processo */
    if(!list_empty(&p->p_child)) {

        /* associa il namespace al figlio */
        pcb_t *child = list_first_entry(&p->p_child, pcb_t, p_child);
        child->namespaces[ns->n_type] = ns;

        /* itera e associa ai fratelli del figlio */
        pcb_t* curr = NULL;
        list_for_each_entry(curr, &child->p_sib, p_sib){
            curr->namespaces[ns->n_type] = ns;
        }
    }
    return TRUE;
}


nsd_t *allocNamespace(int type){
    if(type<0 || type>NS_TYPE_LAST || list_empty(&type_nsFree_h[type])) {
        return NULL;
    }

    /* assegna a un puntatore il namespace da restituire */
    nsd_t *new_nsd = list_first_entry(&type_nsFree_h[type], nsd_t, n_link);

    /* rimuove il namespace dalla lista dei liberi e lo aggiunge alla lista utilizzati */
    list_del(&type_nsFree_h[type]);
    list_add(&new_nsd->n_link, &type_nsList_h[type]);
    return new_nsd;
}



void freeNamespace(nsd_t *ns){
    /* rimuove ns dalla lista degli utilizzati e lo aggiunge alla lista dei liberi */
    list_del(&ns->n_link);
    list_add(&ns->n_link, &type_nsFree_h[ns->n_type]);
}