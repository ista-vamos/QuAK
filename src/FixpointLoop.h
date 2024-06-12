
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
	FixpointLoop (State* initA, TargetOf* initB, unsigned int capacity);
	bool apply ();
	ContextOf* post (ContextOf* currentB, Symbol* symbol);
	bool addIfExtreme (State* stateA, ContextOf* setB, Word* word);
};

#endif /* FIXPOINTLOOP_H_ */
