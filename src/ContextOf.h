
#ifndef CONTEXTOF_H_
#define CONTEXTOF_H_

#include "StateRelation.h"
#include "Map.h"
#include "Weight.h"
#include "State.h"


class ContextOf : protected MapVec<StateRelation*> {
public:
	ContextOf(unsigned int capacity);
	void add (State* fromB, State* toB, Weight<weight_t>* weight);
	bool smaller_than (ContextOf* other);
};

#endif /* CONTEXTOF_H_ */
