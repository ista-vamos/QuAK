
#ifndef TARGETOF_H_
#define TARGETOF_H_

#include "../State.h"
#include "../Set.h"

class TargetOf : protected SetStd<State*> {
public:
	~TargetOf();
	TargetOf();
	void add (State* state);
	bool smaller_than (TargetOf* other);
	unsigned int size () {return this->all.size(); };
	auto begin() {return SetStd<State*>::begin();};
	auto end() {return SetStd<State*>::end();};
};

#endif /* TARGETOF_H_ */

