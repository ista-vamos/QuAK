
#ifndef CONTEXTOF_H_
#define CONTEXTOF_H_

#include "StateRelation.h"
#include "../Map.h"
#include "../State.h"
#include "../Symbol.h"



class ContextOf : protected MapArray<StateRelation*> {
private:
	int nb_ref = 0;
public:
	~ContextOf();
	ContextOf(unsigned int capacity);
	ContextOf(ContextOf* currentB, Symbol* symbol);

	void decreaseRef () { nb_ref = nb_ref - 1; }
	void increaseRef () { nb_ref = nb_ref + 1; }
	int getRef () { return nb_ref; }

	//void print ();

	void add (State* fromB, State* toB, unsigned int weight_id);
	unsigned int size() const {return MapArray<StateRelation*>::size(); };
	bool smaller_than (ContextOf* other, weight_t weight_this, weight_t weight_other);
	StateRelation* at (unsigned int weight_id) const { return MapArray<StateRelation*>::at(weight_id); }
};

#endif /* CONTEXTOF_H_ */
