
#ifndef FIXPOINTSTEM_H_
#define FIXPOINTSTEM_H_

#include "PostTargetVariable.h"
#include "TargetOf.h"
#include "../Word.h"

class FixpointStem {
private:
	bool reversed_inclusion;
	PostTargetVariable* content;
	PostTargetVariable* updates;
	PostTargetVariable* buffer;
public:
	~FixpointStem ();
	FixpointStem (State* initA, State* initB, bool rev);
	bool apply ();
	TargetOf* post (TargetOf* currentB, Symbol* symbol);
	bool addIfExtreme (State* stateA, TargetOf* setB, Word* word);
};

#endif /* FIXPOINTSTEM_H_ */


