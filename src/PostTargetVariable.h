
#ifndef POSTTARGETVARIABLE_H_
#define POSTTARGETVARIABLE_H_

#include "Word.h"
#include "State.h"
#include "Set.h"
#include "Map.h"
#include "TargetOf.h"

class PostTargetVariable : protected MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*> {
public:
	PostTargetVariable();
	void add (State* q, TargetOf* new_target, Word* w);
	bool addIfMin (State* q, TargetOf* new_target, Word* w);
	bool addIfMax (State* q, TargetOf* new_target, Word* w);
	unsigned int size () { return MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>::size(); };
	auto begin () { return MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>::begin(); };
	auto end () { return MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>::end(); };
};

#endif /* POSTTARGETVARIABLE_H_ */




