
#ifndef TARGETOF_H_
#define TARGETOF_H_

#include "../State.h"
#include "../Set.h"

class TargetOf : protected SetStd<State*> {
private:
	int nb_ref = 0;
public:
	~TargetOf();
	TargetOf();
	void decreaseRef () { nb_ref = nb_ref - 1; }
	void increaseRef () { nb_ref = nb_ref + 1; }
	int getRef () { return nb_ref; }

	void add (State* stateB);
	bool smaller_than (TargetOf* other);
	//static std::string toString(TargetOf* tmp);
	//std::string toString () const;
	//unsigned int size () {return this->all.size(); };
	auto begin() {return SetStd<State*>::begin();};
	auto end() {return SetStd<State*>::end();};
};

#endif /* TARGETOF_H_ */

