
#ifndef SET_H_
#define SET_H_

#include <iostream>
#include <string>
#include <set>
#include <list>


/*
template <typename T> class Set {
protected:
	Set();
	~Set();
public:
	void insert(T element);
	void erase(T element);
	unsigned int size();
	std::string toString(std::string (*f) (T element)) const;
};
*/


template <typename T_element> class SetStd {//: public Set<T> {
private:
	std::set<T_element> all;
public:
	SetStd();
	~SetStd();
	void insert(T_element element);
	void erase(T_element element);
	unsigned int size() const;
	std::string toString (std::string (*f) (T_element element)) const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
	auto cbegin() {return all.cbegin();};
	auto cend() {return all.cend();};
};



template <typename T_element> class SetList {//: public Set<T> {
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
	auto cbegin() {return all.cbegin();};
	auto cend() {return all.cend();};
};




#include "Weight.h"
class Symbol;
class State;
class Edge;
template class SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>>;
template class SetStd<std::string>;
template class SetStd<weight_t>;
template class SetStd<Symbol*>;
template class SetStd<State*>;
template class SetStd<Edge*>;

template class SetList<State*>;
template class SetList<Edge*>;

#endif /* SET_H_ */
