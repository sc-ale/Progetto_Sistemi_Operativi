#ifndef NS_H
#define NS_H

#include "pandos_types.h"
#include "types.h"


HIDDEN struct list_head type_nsFree_h[NS_TYPE_MAX];
HIDDEN struct list_head type_nsList_h[NS_TYPE_MAX];
HIDDEN nsd_t type_nsd[MAXPROC][NS_TYPE_MAX];

/**
 * Inizializza tutte le liste dei namespace liberi. Questo metodo viene invocato 
 * una volta sola durante l’inizializzazione della struttura dati
 **/
void initNamespaces();

/* Ritorna il namespace di tipo type associato al processo p (o NULL) */
nsd_t *getNamespace(pcb_t *, int);

/**
 * Associa al processo p e a tutti I suoi fiigli il namespace ns.
 * Ritorna FALSE in caso di errore, TRUE altrimenti
 **/
int addNamespace(pcb_t *, nsd_t *);

/* Alloca un namespace di tipo type dalla lista corretta */
nsd_t *allocNamespace(int);

/* Libera il namespace ns ri-inserendolo nella lista di namespace corretta */
void freeNamespace(nsd_t *);

#endif