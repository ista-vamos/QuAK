
#ifndef CONTEXTOF_H_
#define CONTEXTOF_H_

#include "StateRelation.h"
#include "../Map.h"
#include "../State.h"


class ContextOf : protected MapVec<StateRelation*> {
public:
	~ContextOf();
	ContextOf(unsigned int capacity);
	void add (State* fromB, State* toB, unsigned int weight_id);
	bool smaller_than (ContextOf* other);
	unsigned int size () { return this->size(); };
	StateRelation* at (unsigned int weight_id) const;
};

#endif /* CONTEXTOF_H_ */
