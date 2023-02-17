# PANDA+ PHASE 1


## LISTE
Nelle liste non abbiamo preso scelte degne di nota poiché abbiamo semplicemente seguito la traccia. 

## ALBERI
Per quanto riguarda la scelta implementative delle funzioni sugli alberi
abbiamo deciso che la lista p_child di tipo list_head sia utilizzata come una lista linearly linked, ovvero
p_child.prev punta sempre a se stesso mentre p_child.next punta al figlio nel caso in cui ci sia, altrimenti punta a se stesso. 
Inoltre, abbiamo preferito implementare prima outChild rispetto a removeChild siccome quest'ultima è un caso particolare di outChild. 

## SEMAFORI
Nell'implementazione dei semafori ci siamo limitati a seguire la consegna. E' necessario sottolineare la preferenza ad implementare 
prima la funzione outBlocked siccome questa rappresenta il caso generale di removeBlocked. 

## NAMESPACE
Per gestire i namespace abbiamo utilizzato due array di puntatori per le liste dei namespace liberi e utilizzati e una matrice che
descrive un array per tipo di namespace. Nell'attuale fase 1 viene utilizzato solo NS_PID come tipo di namespace quindi la matrice,
al momento, è un solo array ma, in vista della fase 2, riteniamo sia una scelta che ripagherà sul lungo termine in quanto è già
in grado di gestire più tipi di namespace. 