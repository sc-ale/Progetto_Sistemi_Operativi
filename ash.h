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
        struct semd_t* corrente; 
        hash_for_each_possible(semd_h, corrente, s_link, semAdd){
            p->p_semAdd = semAdd;
            insertProcQ(&corrente->s_procq,p);
            return 0;
        }

        if (!list_empty(&semdFree_h)) {
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
    semd_t *semdP;
    struct list_head *corrente, *temp = NULL;

    hash_for_each_possible(semd_h,semdP,s_link,p->p_semAdd) {
        break;
    }

    list_for_each_safe(corrente, temp, &semdP->s_procq) 
    {
        semd_t *temp = list_entry(corrente, semd_t, s_procq);

        if(&temp->s_procq==&p->p_list) 
        {
            //trovato p da eliminare
            list_del_init(&temp->s_procq);
            if (list_empty(&semdP->s_procq))
            {
                //coda dei semafori bloccati vuota
                hash_del(&semdP->s_link);
                list_add(&semdP->s_freelink,&semdFree_h);
            }
            return p;
        }
    }
    return NULL;
}


/* Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata 
al SEMD della ASH con chiave semAdd. Se tale descrittore non esiste nella ASH,
restituisce NULL. Altrimenti, restituisce l’elemento rimosso.
Se la coda dei processi bloccati per il semaforo diventa vuota,
rimuove il descrittore corrispondente dalla ASH e lo inserisce nella coda dei descrittori liberi (semdFree_h)*/

pcb_t* removeBlocked(int *semAdd)
{
    //dobbiamo controllare se togliendo un pcb dalla coda questa diventa vuota
    //usiamo removeProcQ per rimuovere il pcb dalla coda
    //dobbiamo aggiornare la chiave di quello che eliminiamo
   /*
    struct semd_t *corrente;
    struct hlist_node *temp;
    struct pcb_t *eliminato = NULL;

    hash_for_each_possible_safe(semd_h, corrente, temp, s_link, semAdd)
    {
        //semaforo già in hash
        eliminato = removeProcQ(&corrente->s_procq);

        if(list_empty(&corrente->s_procq)) {
            //dobbiamo rimuovere il semd dalla ash e inserirlo in semdFree_h
                hash_del(temp);
                list_add(&corrente->s_freelink, &semdFree_h);
        }
        eliminato->p_semAdd=NULL;
        break;
    }

    return eliminato; 
    */

    semd_t *it, *temp;
    struct list_head *tmp,*tmp2 = NULL;

    hash_for_each_possible(semd_h,it,s_link,semAdd) {
        break;
    }
    pcb_t *trovato = list_first_entry(&it->s_procq, pcb_t, p_list);
    return outBlocked(trovato);

}

