
#ifndef POSTTARGETVARIABLE_H_
#define POSTTARGETVARIABLE_H_

#include "../Word.h"
#include "../State.h"
#include "../Set.h"
#include "../Map.h"
#include "TargetOf.h"

class PostTargetVariable : protected MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*> {
private:
	void erase (State* stateA, std::pair<TargetOf*, Word*> pair);
public:
	~PostTargetVariable();
	PostTargetVariable();
	void add (State* stateA, TargetOf* setB, Word* w);
	bool addIfMin (State* stateA, TargetOf* setB, Word* w);
	bool addIfMax (State* stateA, TargetOf* setB, Word* w);
	void clear ();
	SetStd<std::pair<TargetOf*,Word*>>* getSetOfTargetsOrNULL (State* stateA);
	unsigned int size () { return MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>::size(); };
	auto begin () { return MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>::begin(); };
	auto end () { return MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>::end(); };
};

#endif /* POSTTARGETVARIABLE_H_ */




