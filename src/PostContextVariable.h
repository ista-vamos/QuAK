/*
 * PostContextVariable.h
 *
 *  Created on: Jun 12, 2024
 *      Author: nmazzocc
 */

#ifndef POSTCONTEXTVARIABLE_H_
#define POSTCONTEXTVARIABLE_H_


#include "Word.h"
#include "State.h"
#include "Set.h"
#include "Map.h"
#include "ContextOf.h"


class PostContextVariable : protected MapStd<State*, SetStd<std::pair<ContextOf*,Word*>>*> {
public:
	~PostContextVariable ();
	PostContextVariable();
	void add (State* stateA, ContextOf* setB, Word* w);
	bool addIfMin (State* stateA, ContextOf* setB, Word* w);
	bool addIfMax (State* stateA, ContextOf* setB, Word* w);
	unsigned int size () { return MapStd<State*,SetStd<std::pair<ContextOf*,Word*>>*>::size(); };
	auto begin () { return MapStd<State*,SetStd<std::pair<ContextOf*,Word*>>*>::begin(); };
	auto end () { return MapStd<State*,SetStd<std::pair<ContextOf*,Word*>>*>::end(); };
};

#endif /* POSTCONTEXTVARIABLE_H_ */
