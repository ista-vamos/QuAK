
#ifndef MONITOR_H_
#define MONITOR_H_

#include "Automaton.h"

class Monitor : protected Automaton {
private:
	weight_t* top_values;
	weight_t* bot_values;
	State* current;
public:
	Monitor(const Automaton* A, value_function_t f);
	void read(Symbol* symbol);
	weight_t getLowest() const;
	weight_t getHighest() const;
};

#endif /* MONITOR_H_ */
