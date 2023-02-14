#include <stdio.h>
#include "hashtable.h"


semd_t semd_table[MAXPROC]; 
LIST_HEAD(semdFree_h); /* Lista dei SEMD liberi o inutilizzati */
DECLARE_HASHTABLE(semd_h, 5); /* Hash per i semafori attivi*/

/* Inizializza la lista dei semdFree_h in modo da contenere tutti gli elementi della semdTable */
void initASH()
{
   for(int i=0; i<MAXPROC; i++){
                list_add(&semd_table[i].s_freelink, &semdFree_h);
    }
}

/* Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave semAdd. 
Se il semaforo corrispondente non è presente nella ASH, alloca un nuovo SEMD dalla lista di quelli liberi 
(semdFree) e lo inserisce nella ASH, settando I campi in maniera opportuna (i.e. key e s_procQ).
Se non è possibile allocare un nuovo SEMD perché la lista di quelli liberi è vuota, restituisce TRUE. 
In tutti gli altri casi, restituisce FALSE*/
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

/*Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da p->p_semAdd). 
Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore). 
Altrimenti, restituisce p. Se la coda dei processi bloccati per il semaforo diventa vuota,
rimuove il descrittore corrispondente dalla ASH e lo inserisce nella coda dei descrittori liberi*/
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
    return NULL;
    }
}

/* Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata 
al SEMD della ASH con chiave semAdd. Se tale descrittore non esiste nella ASH,
restituisce NULL. Altrimenti, restituisce l’elemento rimosso.
Se la coda dei processi bloccati per il semaforo diventa vuota,
rimuove il descrittore corrispondente dalla ASH e lo inserisce nella coda dei descrittori liberi (semdFree_h)*/
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

/* Restituisce (senza rimuovere) il puntatore al PCB che si trova in
 testa alla coda dei processi associata al SEMD con chiave semAdd. 
 Ritorna NULL se il SEMD non compare nella ASH oppure se compare ma la sua coda dei processi è vuota */
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
