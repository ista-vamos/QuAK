/*
 * PostContextVariable.h
 *
 *  Created on: Jun 12, 2024
 *      Author: nmazzocc
 */

#ifndef POSTCONTEXTVARIABLE_H_
#define POSTCONTEXTVARIABLE_H_


#include "ContextOf.h"
#include "../Word.h"
#include "../State.h"
#include "../Set.h"
#include "../Map.h"


class PostContextVariable : protected MapStd<State*, SetStd<std::pair<ContextOf*,std::pair<Word*, weight_t>>>*> {
private:
	void erase (State* stateA, std::pair<ContextOf*, std::pair<Word*,weight_t>> pair);
public:
	~PostContextVariable ();
	PostContextVariable();
	void add (State* stateA, ContextOf* setB, Word* w, weight_t value);
	bool addIfMin (State* stateA, ContextOf* setB, Word* w, weight_t value);
	//bool addIfMax (State* stateA, ContextOf* setB, Word* w, weight_t value);
	void clear ();
	SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>* getSetOfContextsOrNULL (State* stateA);
	unsigned int size () { return MapStd<State*,SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*>::size(); };
	auto begin () { return MapStd<State*,SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*>::begin(); };
	auto end () { return MapStd<State*,SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*>::end(); };
};

#endif /* POSTCONTEXTVARIABLE_H_ */
