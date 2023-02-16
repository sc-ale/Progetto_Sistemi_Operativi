#ifndef NS_H
#define NS_H

#include "pandos_types.h"
#include "list.h"
#include "types.h"


/*Inizializza le liste di namespace liberi*/
void initNamespaces();

/*Ritorna il namespace di tipo type associato al processo p*/
nsd_t *getNamespace(pcb_t *p, int type);

/*Associa al namespace p e a tutti i suoi il namespace ns
Ritorna FALSE in caso di errore*/
int addNamespace(pcb_t *p, nsd_t *ns);

/*Alloca un namespace di tipo type dalla lista relativa dei namespace liberi*/
nsd_t *allocNamespace(int type);

/*Libera il namespace ns reinserendolo nella lista dei namespace liberi*/
void freeNamespace(nsd_t *ns);

#endif