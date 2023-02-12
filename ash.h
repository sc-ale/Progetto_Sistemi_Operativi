#include <stdio.h>
#include "pandos_types.h"
#include "hashtable.h"
#include "pcb.h"

semd_t semd_table[MAXPROC];
HLIST_HEAD(semdFree_h); /* Lista dei SEMD liberi o inutilizzati */
DEFINE_HASHTABLE(semd_h, 5); /* Hash per i semafori attivi*/


/* Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. 
Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati */
void initASH(){
    for(int i=0; i<MAXPROC; i++){

    }
}

