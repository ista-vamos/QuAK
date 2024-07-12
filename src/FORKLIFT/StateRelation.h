
#ifndef STATERELATION_H_
#define STATERELATION_H_

#include "TargetOf.h"
#include "../Map.h"

class StateRelation : protected MapStd<State*,TargetOf*> {
public:
	~StateRelation();
	StateRelation();

	void add (State* fromB, State* toB);
	bool smaller_than (StateRelation* other);

	//void print ();

	auto begin() {return MapStd<State*,TargetOf*>::begin();};
	auto end() {return MapStd<State*,TargetOf*>::end();};
	bool contains (State* state) { return MapStd<State*,TargetOf*>::contains(state); }
	TargetOf* at (State* state) { return MapStd<State*,TargetOf*>::at(state); }
};


#endif /* STATERELATION_H_ */
