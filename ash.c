#include "ash.h"

HIDDEN semd_t semd_table[MAXPROC]; 
HIDDEN LIST_HEAD(semdFree_h);
HIDDEN DECLARE_HASHTABLE(semd_h, 5);

void initASH()
{
   for(int i=0; i<MAXPROC; i++){
                list_add(&semd_table[i].s_freelink, &semdFree_h);
    }
}


int insertBlocked(int* semAdd, pcb_t* p){
    if(p->p_semAdd==NULL) {
        /* p non è associato ad altri semafori */
        struct semd_t* corrente; 
        hash_for_each_possible(semd_h, corrente, s_link, semAdd){
            /* semaforo già in semd_h */
            p->p_semAdd = semAdd;
            insertProcQ(&corrente->s_procq,p);
            return 0;
        }

        if (!list_empty(&semdFree_h)) {
            /* allochiamo un semd libero per p e lo inseriamo in semd_h */
            semd_t* newSemd = list_first_entry(&semdFree_h,semd_t,s_freelink);

            newSemd->s_key = semAdd;
            mkEmptyProcQ(&newSemd->s_procq); 
            p->p_semAdd = semAdd; 
            insertProcQ(&newSemd->s_procq, p);

            hash_add(semd_h,&newSemd->s_link, newSemd->s_key);
            list_del(&newSemd->s_freelink);
            return 0;
        }
    }
    return 1;
}


pcb_t* outBlocked(pcb_t *p) {
    semd_t *semdP=NULL;
    struct list_head *corrente, *temp = NULL;

    hash_for_each_possible(semd_h,semdP,s_link,p->p_semAdd) {
        /* se entriamo nel ciclo semdP punta al semaforo che blocca il processo p */
        break;
    }

    if(semdP!=NULL){

        list_for_each_safe(corrente, temp, &semdP->s_procq) 
        {
            pcb_t *eliminato = list_entry(corrente, pcb_t, p_list);

            if(eliminato==p) 
            {
                /* trovato il processo da eliminare */
                list_del_init(&eliminato->p_list);
                if (list_empty(&semdP->s_procq))
                {
                    /* la coda dei semafori bloccati è vuota quindi inseriamo semdP in semdFree_h */
                    hash_del(&semdP->s_link);
                    list_add(&semdP->s_freelink,&semdFree_h);
                    p->p_semAdd = NULL;
                }
                return p;
            }
        }
    }
    return NULL;
}


pcb_t* removeBlocked(int *semAdd)
{
    semd_t *semdP=NULL;

    hash_for_each_possible(semd_h,semdP,s_link,semAdd) {
        /* se entriamo nel ciclo semdP punta al semaforo con chiave semAdd */
        break;
    }

    if(semdP!=NULL) {
        /* eliminiamo il primo processo in semdP */
        pcb_t *trovato = list_first_entry(&semdP->s_procq, pcb_t, p_list);
        return outBlocked(trovato);
    }
    return NULL;
}


pcb_t* headBlocked(int *semAdd)
{
    semd_t *semdP=NULL;

    hash_for_each_possible(semd_h,semdP,s_link,semAdd) {
         /* se entriamo nel ciclo semdP punta al semaforo con chiave semAdd */
        break;
    }

    if(semdP!=NULL) {
        /* se la coda dei processi non è vuota ritorniamo il pcb in testa alla coda di semdP */
        if(!list_empty(&semdP->s_procq))
            return list_first_entry(&semdP->s_procq, pcb_t, p_list);
    }

    return NULL;
}