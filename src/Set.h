
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


template <typename T> class SetStd {//: public Set<T> {
private:
	std::set<T> all;
public:
	SetStd();
	~SetStd();
	void insert(T element);
	void erase(T element);
	unsigned int size() const;
	std::string toString(std::string (*f) (T element)) const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
	auto cbegin() {return all.cbegin();};
	auto cend() {return all.cend();};
};



template <typename T_value> class SetList {//: public Set<T> {
private:
	std::list<T_value> all;
public:
	SetList();
	~SetList();
	void insert(T_value element);
	unsigned int size() const;
	std::string toString(std::string (*f) (T_value element)) const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
	auto cbegin() {return all.cbegin();};
	auto cend() {return all.cend();};
};



class Symbol;
class State;
class Edge;
template class SetStd<std::pair<std::pair<std::string, int>,std::pair<std::string, std::string>>>;
template class SetStd<std::string>;
template class SetStd<int>;
template class SetStd<Symbol*>;
template class SetStd<State*>;
template class SetStd<Edge*>;
template class SetList<State*>;


#endif /* SET_H_ */
