
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
	//unsigned int size () { return MapStd<State*,TargetOf*>::size(); }; //fixme: remove
	auto begin() {return MapStd<State*,TargetOf*>::begin();};
	auto end() {return MapStd<State*,TargetOf*>::end();};
};

#endif /* STATERELATION_H_ */
