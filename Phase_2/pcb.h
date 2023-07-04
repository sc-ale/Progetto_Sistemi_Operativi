#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"
#include "pandos_const.h"
#include "list.h"


/* Inizializzazione di pcbFree_h inserendo gli elementi in pcbFree_table */
void initPcbs();

/* Inserisce il PCB puntato da p in pcbFree_h */
void freePcb(pcb_t *);

/**
 * Restituisce NULL se la pcbFree_h è vuota. Altrimenti rimuove un elemento dalla pcbFree, 
 * inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso 
 **/
pcb_t *allocPcb();

/* crea una lista di pcb inizializzandola come vuota */
void mkEmptyProcQ(struct list_head *);

/* Restrituisce true se la lista puntata da head è vuota, false altrimenti */
int emptyProcQ(struct list_head *);

/* Inserisce l’elemento puntato da p nella coda dei processi puntata da head */
void insertProcQ(struct list_head *, pcb_t*);

/**
 * Restituisce l’elemento di testa di head senza rimuoverlo.
 * Ritorna NULL se la coda è vuota
 **/
pcb_t *headProcQ(struct list_head *);

/**
 * Rimuove il primo elemento dalla coda dei processi puntata da head. 
 * Ritorna NULL se la coda è vuota.
 * Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.
 **/
pcb_t *removeProcQ(struct list_head *);

/** 
 * Rimuove il PCB puntato da p dalla coda dei processi puntata da head.
 * Se p non è presente nella coda, restituisce NULL. 
 * (NOTA: p può trovarsi in una posizione arbitraria della coda). 
 **/
pcb_t *outProcQ(struct list_head*, pcb_t *);

/* Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti */
int emptyChild(pcb_t *);

/* Inserisce il PCB puntato da p come figlio del PCB puntato da prnt */
void insertChild(pcb_t *, pcb_t *);

/** 
 * Rimuove il PCB puntato da p dalla lista dei figli del padre. 
 * Se il PCB puntato da p non ha un padre, restituisce NULL, altrimenti restituisce l’elemento 
 * rimosso (cioè p). A differenza della removeChild, p può trovarsi in una posizione arbitraria 
 * (ossia non è necessariamente il primo figlio del padre, in questo caso p è il figlio che va rimosso) 
 **/
pcb_t *outChild(pcb_t *);

/* Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL */
pcb_t *removeChild(pcb_t *);

/* Ritorna il puntatore al PCB con p_pid == pid che si trova nella lista head */
pcb_t *getProcByPid(int, struct list_head*);

#endif