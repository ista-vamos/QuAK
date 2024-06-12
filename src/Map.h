

#ifndef MAP_H_
#define MAP_H_

#include <vector>
#include <string>
#include <map>



template <typename T_key, typename T_value>
class MapStd {
private:
	std::map<T_key, T_value> all;
public:
	MapStd();
	~MapStd();
	void insert(T_key key, T_value value);
	unsigned int size () const;
	T_value at (T_key key);
	bool contains (T_key key) { return all.find(key) != all.end();};
	void update (T_key key, T_value value);
	std::string toString(std::string (*f_key) (T_key key), std::string (*f_value) (T_value value)) const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
};




template <typename T_value>
class MapVec {
private:
	T_value* all = NULL;
	unsigned int capacity = 0;
public:
	~MapVec();
	MapVec(unsigned int capacity);
	void insert(unsigned int key, T_value value);
	unsigned int size () const;
	T_value at (unsigned int key) const;
	std::string toString(std::string (*f_value) (T_value value)) const;


	/*
	class Iterator {
	public:
		T_value* iter;
	public:
		Iterator(T_value* iter) : iter(iter) {}
		T_value& operator*() const { return *iter; }
		T_value* operator->() { return iter; }
		Iterator& operator++() { iter++; return *this; }
	    friend bool operator== (const Iterator& a, const Iterator& b) { return a.iter == b.iter; };
	    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.iter != b.iter; };
	};
	Iterator begin() { return Iterator(&all[0]); }
	Iterator end()   { return Iterator(&all[capacity]); }
	class ConstIterator {
	public:
		T_value* iter;
	public:
		ConstIterator(T_value* iter) : iter(iter) {}
		const T_value& operator*() const { return *iter; }
		T_value* operator->() { return iter; }
		ConstIterator& operator++() { iter++; return *this; }
		friend bool operator== (const ConstIterator& a, const ConstIterator& b) { return a.iter == b.iter; };
		friend bool operator!= (const ConstIterator& a, const ConstIterator& b) { return a.iter != b.iter; };
	};
	ConstIterator cbegin() const { return ConstIterator(&all[0]); }
	ConstIterator cend() const { return ConstIterator(&all[capacity]); }
	*/
};



#include "Weight.h"
class Symbol;
class Word;
class State;
class TargetOf;
class ContextOf;
class StateRelation;
class Edge;
template <class T> class SetStd;
template <class T> class SetList;

template class MapStd<std::string, Symbol*>;
template class MapStd<weight_t, Weight<weight_t>*>;
template class MapStd<std::string, State*>;
template class MapStd<std::pair<std::string, std::string>,int>;
template class MapStd<std::string, unsigned int>;
template class MapStd<std::string, MapStd<std::string, unsigned int>*>;
template class MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>;
template class MapStd<State*, TargetOf*>;
template class MapStd<State*, SetStd<std::pair<ContextOf*,Word*>>*>;

template class MapVec<Weight<weight_t>*>;
template class MapVec<State*>;
template class MapVec<SetList<Edge*>*>;
template class MapVec<Symbol*>;
template class MapVec<SetStd<Edge*>*>;
template class MapVec<StateRelation*>;


#endif /* MAP_H_ */
