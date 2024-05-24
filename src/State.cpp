
#include "State.h"
#include "utility.h"


int ID_of_States = 0;


State::~State () {
	delete_verbose("@Memory: State deleted\n");
	for (auto edge : *edges) {
		delete edge;
	}
	delete_verbose("@Detail: 1 SetStd will be deleted (state)\n");
	delete edges;
	for (unsigned int id = 0; id < successors->size(); ++id) {
		delete successors->at(id);
	}
	delete successors;
}


State::State (std::string name, unsigned int alphabet_size) :
		id(ID_of_States++),
		name(name),
		scc_tag(-1),
		edges(NULL),
		successors(NULL)
{
	this->edges = new SetStd<Edge*>();
	this->successors = new MapVec<SetStd<Edge*>*>(alphabet_size);
}


std::string State::getName() const {
	return this->name;
}


const int State::getId() const {
	return this->id;
}

const int State::getTag() const {
	return this->scc_tag;
}

void State::setTag(int tag) {
	this->scc_tag = tag;
}

SetStd<Edge*>* State::getEdges() const {
	return this->edges;
}


MapVec<SetStd<Edge*>*>* State::getSuccessors() const {
	return this->successors;
}


void State::addEdge (Edge *edge) {
	this->edges->insert(edge);
	addSuccessor(edge);
}


void State::addSuccessor (Edge* edge) {
	Symbol* symbol = edge->getSymbol();
	SetStd<Edge*>* set = this->successors->at(symbol->getId());
	if (set == NULL) {
		set = new SetStd<Edge*>();
		set->insert(edge);
		successors->insert(symbol->getId(), set);
	}
	else {
		set->insert(edge);
	}
}


std::string State::State::toString(State *state) {
	return state->toString();
}


std::string State::toString() const {
	return this->name;
}


/*
int State::cmp (const State* other) const { return this->id - other->id; }
bool State::operator< (const State* other) const { return this->cmp(other) < 0;}
bool State::operator<= (const State* other) const { return this->cmp(other) <= 0;}
bool State::operator> (const State* other) const { return this->cmp(other) > 0;}
bool State::operator>= (const State* other) const { return this->cmp(other) >= 0;}
bool State::operator== (const State* other) const { return this->cmp(other) == 0; }
bool State::operator!= (const State* other) const { return this->cmp(other) != 0; }
*/


