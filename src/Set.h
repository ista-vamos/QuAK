
#ifndef SET_H_
#define SET_H_

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <unordered_set>

template <typename T_element> class SetStd {
private:
	std::set<T_element> all;
protected:
	void clear () { all.clear(); };
public:
	SetStd();
	~SetStd();
	void insert(T_element element);
	void erase(T_element element);
	bool contains (T_element element) { return all.find(element) != all.end(); };
	unsigned int size() const;
	std::string toString (std::string (*f) (T_element element)) const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
};



template <typename T_element> class SetList {
private:
	std::list<T_element> all;
public:
	SetList();
	~SetList();
	void push(T_element element);
	void queue(T_element element);
	void pop();
	T_element head();
	unsigned int size() const;
	std::string toString (std::string (*f) (T_element element)) const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
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

template class SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>>;
template class SetStd<std::string>;
template class SetStd<weight_t>;
template class SetStd<Symbol*>;
template class SetStd<State*>;
template class SetStd<Word*>;
template class SetStd<Edge*>;
template class SetStd<std::pair<TargetOf*,Word*>>;
template class SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>;
template class SetStd<std::pair<State*, unsigned int>>;


template class SetList<SCC_Tree*>;
template class SetList<State*>;
template class SetList<Edge*>;

#endif /* SET_H_ */
