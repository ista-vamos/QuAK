#ifndef INCLUSION_H_
#define INCLUSION_H_


#include "../Automaton.h"


// These are defined by CMake. Uncomment only if you want
// to hard-code these (or you don't use CMake to create makefiles)
// #define INCLUSION_SCC_SEARCH_ACTIVE
// #define CONTEXT_REDUNDANCY_ACTIVE


bool inclusion (const Automaton* A, const Automaton* B);
void debug_test();
void debug_test2();


#endif /* INCLUSION_H_ */
