#ifndef INCLUSION_H_
#define INCLUSION_H_


#include "../Automaton.h"


// These are defined by CMake or Makefile.legacy. Uncomment only if you want
// to (temporarily, e.g., for debugging without changing makefiles)
// hard-code these.
// #define INCLUSION_SCC_SEARCH_ACTIVE
 #define CONTEXT_REDUNDANCY_ACTIVE


bool inclusion (const Automaton* A, const Automaton* B);
void debug_test();
void debug_test2();


#endif /* INCLUSION_H_ */
