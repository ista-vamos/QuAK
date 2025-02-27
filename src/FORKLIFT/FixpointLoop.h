
#ifndef FIXPOINTLOOP_H_
#define FIXPOINTLOOP_H_

#include "PostContextVariable.h"

class FixpointLoop {
private:
	unsigned int capacity;
	PostContextVariable* content;
	PostContextVariable* updates;
	PostContextVariable* buffer;
public:
	~FixpointLoop ();
	FixpointLoop (Symbol* initSymbol, State* initA, TargetOf* initB, unsigned int capacity);
	bool apply ();
	SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>* getSetOfContextsOrNULL (State* stateA);
	ContextOf* post (ContextOf* currentB, Symbol* symbol);
	bool addIfExtreme (State* stateA, ContextOf* setB, Word* word, weight_t value);
};

#endif /* FIXPOINTLOOP_H_ */
