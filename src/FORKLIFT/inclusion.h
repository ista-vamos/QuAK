#ifndef INCLUSION_H_
#define INCLUSION_H_


#include "../Automaton.h"


bool inclusion (const Automaton* A, const Automaton* B, UltimatelyPeriodicWord** witness = nullptr);
void debug_test();
void debug_test2();


#endif /* INCLUSION_H_ */
