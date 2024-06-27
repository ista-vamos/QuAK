

#ifndef INCLUSION_H_
#define INCLUSION_H_


#include "../Automaton.h"


#define INCLUSION_SCC_SEARCH_ACTIVE
#define CONTEXT_REDUNDANCY_ACTIVE



bool inclusion (const Automaton* A, const Automaton* B);
void debug_test();


#endif /* INCLUSION_H_ */
