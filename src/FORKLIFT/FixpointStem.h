
#ifndef FIXPOINTSTEM_H_
#define FIXPOINTSTEM_H_

#include "PostTargetVariable.h"
#include "TargetOf.h"
#include "../Word.h"

class FixpointStem {
public://fixme
	unsigned int nb_constructed = 0;
	unsigned int nb_deleted = 0;
private:
	bool reversed_inclusion;
	PostTargetVariable* content;
	PostTargetVariable* updates;
	PostTargetVariable* buffer;
public:
	~FixpointStem ();
	FixpointStem (State* initA, State* initB, bool rev);
	bool apply ();
	SetStd<std::pair<TargetOf*,Word*>>* getSetOfTargetsOrNULL (State* stateA);
	TargetOf* post (TargetOf* currentB, Symbol* symbol);
	bool addIfExtreme (State* stateA, TargetOf* setB, Word* word);
};

#endif /* FIXPOINTSTEM_H_ */


