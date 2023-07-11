#include "ash.h"

/* variabili globali */
HIDDEN semd_t semd_table[MAXPROC]; 
HIDDEN LIST_HEAD(semdFree_h);
HIDDEN DECLARE_HASHTABLE(semd_h, 5);


void initASH()
{
   for(int i=0; i<MAXPROC; i++){
                list_add(&semd_table[i].s_freelink, &semdFree_h);
    }
}


int insertBlocked(int* semAdd, pcb_t* p)
{
    if(p->p_semAdd==NULL) {
        /* p non è associato ad altri semafori */
        semd_t* corrente = NULL; 
        int bkt;

        hash_for_each(semd_h, bkt, corrente, s_link){
            if(corrente->s_key == semAdd) {
                /*  aggiungiamo p al semaforo corrente, questo if viene effettuato 
                    se esiste già un altro processo associato al semaforo semAdd */
                p->p_semAdd = semAdd;
                insertProcQ(&corrente->s_procq, p);
                return FALSE;
            }
        }

        if (!list_empty(&semdFree_h)) {
            /* allochiamo un semd libero per p e lo inseriamo in semd_h */
            semd_t* newSemd = list_first_entry(&semdFree_h, semd_t, s_freelink);

            newSemd->s_key = semAdd;
            mkEmptyProcQ(&newSemd->s_procq); 
            p->p_semAdd = semAdd; 
            insertProcQ(&newSemd->s_procq, p);

            list_del(&newSemd->s_freelink);
            hash_add(semd_h, &newSemd->s_link, (u32)newSemd->s_key);
            return FALSE;
        }
    } 
    return TRUE;
}


pcb_t* outBlocked(pcb_t *p)
{
    semd_t *semdP=NULL;
    int bkt;
    pcb_t *eliminato = NULL;

    hash_for_each(semd_h, bkt, semdP, s_link) {
        /* se entriamo nel ciclo semdP punta al semaforo che blocca il processo p */
        if(semdP->s_key == p->p_semAdd){
            eliminato = outProcQ(&semdP->s_procq,p);
            eliminato->p_semAdd = NULL;
            removeEmptySemd(semdP);
            break;
        }
    }

    return eliminato;
}



void removeEmptySemd(semd_t* s) {
    if(emptyProcQ(&s->s_procq)) {
        hash_del(&s->s_link);
        list_add(&s->s_freelink,&semdFree_h);
    }
}


pcb_t* removeBlocked(int *semAdd)
{
    semd_t *semdP=NULL;
    int bkt;
    hash_for_each(semd_h,bkt,semdP,s_link) {
        /* se entriamo nel ciclo semdP punta al semaforo con chiave semAdd */
        if (semdP->s_key == semAdd ) {
            pcb_t *trovato = list_first_entry(&semdP->s_procq, pcb_t, p_list);
            return outBlocked(trovato);
        }
    }

    return NULL;
}


pcb_t* headBlocked(int *semAdd)
{
    semd_t *semdP=NULL;
    int bkt;
    hash_for_each(semd_h, bkt, semdP,s_link) {
        /* se entriamo nel ciclo semdP punta al semaforo con chiave semAdd */
        if (semdP->s_key == semAdd ) {
            return headProcQ(&semdP->s_procq);
        }
    }

    return NULL;
}

pcb_t* getProcByPidOnSem(int pid) {
    int bkt;
    semd_t* semdP = NULL;
    pcb_t* proc2rtrn = NULL;

    hash_for_each(semd_h, bkt, semdP, s_link) {
        /* semdP è il cursore per scorrere i semafori */
        proc2rtrn = getProcInHead(pid, &semdP->s_procq);
        if(proc2rtrn != NULL) break;
    }
    
    return proc2rtrn;
}