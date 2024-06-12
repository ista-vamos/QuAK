
#ifndef POSTSTEM_H_
#define POSTSTEM_H_

#include "PostTargetVariable.h"
#include "TargetOf.h"

class PostStem {
public:
	bool reversed_inclusion;
	PostTargetVariable* content;
	PostTargetVariable* updates;
	PostTargetVariable* buffer;

	PostStem (State* initA, State* initB, bool rev);
	bool apply ();
	TargetOf* post (TargetOf* current, Symbol* symbol);
	bool addIfExtreme (State* stateA, TargetOf* postB, Word* word);
};

#endif /* POSTSTEM_H_ */


