#include <stdio.h>
#include "pandos_types.h"
#include "hashtable.h"

semd_t semd_table[MAXPROC];
LIST_HEAD(semdFree_h);
//DEFINE_HASHTABLE(semd_h, 5);


