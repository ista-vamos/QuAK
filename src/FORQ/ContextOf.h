
#ifndef CONTEXTOF_H_
#define CONTEXTOF_H_

#include "StateRelation.h"
#include "../Map.h"
#include "../State.h"


class ContextOf : protected MapVec<StateRelation*> {
private:
	int nb_ref = 0;
public:
	~ContextOf();
	ContextOf(unsigned int capacity);
	void decreaseRef () { nb_ref = nb_ref - 1; }
	void increaseRef () { nb_ref = nb_ref + 1; }
	int getRef () { return nb_ref; }

	void add (State* fromB, State* toB, unsigned int weight_id);
	bool smaller_than (ContextOf* other);
	StateRelation* at (unsigned int weight_id) const { return MapVec<StateRelation*>::at(weight_id); }
	//unsigned int size () { return this->size(); };
};

#endif /* CONTEXTOF_H_ */
