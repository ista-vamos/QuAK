
#ifndef SET_H_
#define SET_H_

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <list>
#include "hash.h"


template <typename T_element> class SetSorted {
private:
	std::set<T_element> all;
public:
	SetSorted();
	~SetSorted();
	void insert(T_element element);
	unsigned int size() const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
	T_element getMin () { return *(all.begin());};
	T_element getMax () { return *(all.rbegin());};
};



template <typename T_element> class SetStd {
private:
	std::set<T_element> all;
	//std::unordered_set<T_element> all;
public:
	SetStd();
	~SetStd();
	void insert(T_element element);
	void erase(T_element element);
	bool contains (T_element element) { return all.find(element) != all.end(); };
	unsigned int size() const;
	void clear () { all.clear(); };
	auto begin() {return all.begin();};
	auto end() {return all.end();};
};



template <typename T_element> class SetList {
private:
	std::list<T_element> all;
public:
	SetList(SetList<T_element>* to_copy);
	SetList();
	~SetList();
	void push(T_element element);
	void pop();
	T_element head();
	unsigned int size() const;
	auto begin() { return all.begin(); };
	auto end() { return all.end(); };
};





#include "Weight.h"
class SCC_Tree;
class Symbol;
class Word;
class ContextOf;
class StateRelation;
class TargetOf;
class State;
class Edge;


template class SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>>; // Parser
template class SetStd<std::string>; // Parser
template class SetStd<Symbol*>; // State
template class SetStd<State*>; // State
template class SetStd<Word*>;
template class SetStd<Edge*>;
template class SetStd<std::pair<TargetOf*,Word*>>;
template class SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>;
template class SetStd<std::pair<State*, unsigned int>>; // Automaton
template class SetStd<std::pair<State*,std::pair<unsigned int, bool>>>;
template class SetStd<std::pair<State*, Weight*>>;
template class SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>>;

template class SetSorted<weight_t>; // Parser, Automaton

template class SetList<SCC_Tree*>;
template class SetList<State*>;
template class SetList<Edge*>;

#endif /* SET_H_ */
