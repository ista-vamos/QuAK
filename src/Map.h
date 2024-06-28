

#ifndef MAP_H_
#define MAP_H_

#include <vector>
#include <string>
#include <map>
#include <unordered_map>


template <typename T_key, typename T_value>
class MapStd {
private:
	//std::unordered_map<T_key, T_value> all;
	std::map<T_key, T_value> all;
public:
	MapStd();
	~MapStd();
	void insert(T_key key, T_value value);
	unsigned int size () const;
	T_value at (T_key key);
	bool contains (T_key key) { return all.find(key) != all.end(); };
	void update (T_key key, T_value value);
	std::string toString(std::string (*f_key) (T_key key), std::string (*f_value) (T_value value)) const;
	void clear () { all.clear(); }
	auto begin() {return all.begin();};
	auto end() {return all.end();};
};




template <typename T_value>
class MapArray {
private:
	T_value* all = NULL;
	unsigned int capacity = 0;
public:
	~MapArray();
	MapArray(unsigned int capacity);
	void insert(unsigned int key, T_value value);
	unsigned int size () const;
	T_value at (unsigned int key) const;
	std::string toString(std::string (*f_value) (T_value value)) const;
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


template class MapStd<std::string, Symbol*>; // Automata
template class MapStd<weight_t, Weight*>; // Automata
template class MapStd<std::string, State*>; // Automata
template class MapStd<State*, SetStd<std::pair<TargetOf*,Word*>>*>; // StateRelation
template class MapStd<State*, TargetOf*>; // PostTargetVariable
template class MapStd<State*, SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*>; // PostContextVariable
template class MapStd<std::pair<State*, Weight*>, State*>;


template class MapArray<Weight*>;
template class MapArray<State*>;
template class MapArray<SetList<Edge*>*>;
template class MapArray<Symbol*>;
template class MapArray<SetStd<Edge*>*>; // State
template class MapArray<StateRelation*>;


#endif /* MAP_H_ */
