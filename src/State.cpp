
#include "State.h"
#include "Symbol.h"
#include "utility.h"


int ID_of_States = 0;
void State::State::RESET() { ID_of_States = 0; }


State::~State () {
	delete_verbose("@Memory: State deletion start (%s)\n", this->toString().c_str());

	delete_verbose("@Detail: 1 SetStd (alphabet) will be deleted (state %s)\n", this->toString().c_str());
	delete alphabet;

	delete_verbose("@Detail: 1 SetStd (edges) will be deleted (state %s)\n", this->toString().c_str());
	delete edges;

	delete_verbose("@Detail: %u SetStd (successors) will be deleted (state %s)\n", this->successors->size(), this->toString().c_str());
	for (unsigned int symbol_id = 0; symbol_id  < this->successors->size(); ++symbol_id ) {
		delete successors->at(symbol_id);
	}
	delete_verbose("@Detail: 1 MapVec (successors) will be deleted (state %s)\n", this->toString().c_str());
	delete successors;

	delete_verbose("@Detail: %u SetStd (predecessors) will be deleted (state %s)\n", this->predecessors->size(), this->toString().c_str());
	for (unsigned int symbol_id = 0; symbol_id  < this->predecessors->size(); ++symbol_id ) {
		delete predecessors->at(symbol_id);
	}
	delete_verbose("@Detail: 1 MapVec (predecessors) will be deleted (state %s)\n", this->toString().c_str());
	delete predecessors;

	delete_verbose("@Memory: State deletion finish (%s)\n", this->toString().c_str());
}


State::State (std::string name, unsigned int alphabet_size) :
		my_id(ID_of_States++),
		name(name),
		my_scc(-1),
		alphabet(NULL),
		edges(NULL),
		successors(NULL),
		predecessors(NULL)
{
	this->alphabet = new SetStd<Symbol*>();
	this->edges = new SetStd<Edge*>();
	this->successors = new MapVec<SetStd<Edge*>*>(alphabet_size);
	this->predecessors = new MapVec<SetStd<Edge*>*>(alphabet_size);
	for (unsigned int symbol_id = 0; symbol_id < alphabet_size; ++symbol_id) {
		this->successors->insert(symbol_id, new SetStd<Edge*>()); //fixme: change later
		this->predecessors->insert(symbol_id, new SetStd<Edge*>());
	}
}

State::State (State* state) :
		my_id(state->my_id),
		name(state->name),
		my_scc(state->my_scc),
		alphabet(NULL),
		edges(NULL),
		successors(NULL),
		predecessors(NULL)
{
	this->alphabet = new SetStd<Symbol*>();
	this->edges = new SetStd<Edge*>();
	this->successors = new MapVec<SetStd<Edge*>*>(state->successors->size());
	this->predecessors = new MapVec<SetStd<Edge*>*>(state->predecessors->size());
	for (unsigned int symbol_id = 0; symbol_id < this->successors->size(); ++symbol_id) {
		this->successors->insert(symbol_id, new SetStd<Edge*>());
		this->predecessors->insert(symbol_id, new SetStd<Edge*>());
	}
}


std::string State::getName() const {
	return this->name;
}


const int State::getId() const {
	return this->my_id;
}

const int State::getTag() const {
	return this->my_scc;
}

void State::setTag(int tag) {
	this->my_scc = tag;
}

SetStd<Symbol*>* State::getAlphabet () const {
	return this->alphabet;
}


SetStd<Edge*>* State::getEdges () const {
	return this->edges;
}


SetStd<Edge*>* State::getSuccessors(unsigned int symbol_id) const {
	return this->successors->at(symbol_id);
}


SetStd<Edge*>* State::getPredecessors(unsigned int symbol_id) const {
	return this->predecessors->at(symbol_id);
}



void State::addEdge (Edge *edge) {
	this->edges->insert(edge);
}


void State::addSuccessor (Edge* edge) {
	this->alphabet->insert(edge->getSymbol());
	this->successors->at(edge->getSymbol()->getId())->insert(edge);
}


void State::addPredecessor (Edge* edge) {
	this->predecessors->at(edge->getSymbol()->getId())->insert(edge);
}


std::string State::State::toString(State *state) {
	return state->toString();
}


std::string State::toString() const {
	return this->name + ", scc: " + std::to_string(this->my_scc);
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


