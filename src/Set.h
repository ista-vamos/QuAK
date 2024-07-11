#ifndef QUAK_SET_H_
#define QUAK_SET_H_

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
	void insert(T_element element) { this->all.insert(element); }
	unsigned int size() const { return this->all.size(); }

	auto begin() -> auto {return all.begin();};
	auto end() -> auto {return all.end();};
	T_element getMin () { return *(all.begin());};
	T_element getMax () { return *(all.rbegin());};
};



template <typename T_element> class SetStd {
private:
	std::set<T_element> all;
	//std::unordered_set<T_element> all;
public:
	void insert(T_element element) { this->all.insert(element); }
	void erase(T_element element) {  this->all.erase(element); }
	bool contains (T_element element) { return all.count(element) > 0; };

	std::string toString (std::string (*f) (T_element element)) const {
    std::string s = "";
    for (T_element e : this->all){
    	s.append("\n\t\t");
    	s.append(f(e));
    }
    
    return s;
  }

	unsigned int size() const { return this->all.size(); }
	void clear () { all.clear(); };

	auto begin() -> auto { return all.begin();};
	auto end() -> auto { return all.end();};
};



template <typename T_element> class SetList {
private:
	std::list<T_element> all;
public:
	SetList(SetList<T_element>* to_copy) : all(to_copy->all) {}
	SetList() = default;
	void push(T_element element) { all.push_front(element); }
	void pop() { all.erase(all.begin()); }
	T_element head() { return *(all.begin()); };
	unsigned int size() const { return all.size(); }

	auto begin() -> auto { return all.begin(); };
	auto end() -> auto { return all.end(); };
};

#endif /* QUAK_SET_H_ */
