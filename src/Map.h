#ifndef QUAK_MAP_H_
#define QUAK_MAP_H_

#include <vector>
#include <string>
#include <map>
#include <cassert>
#include <cstring>

#include "hash.h"


template <typename T_key, typename T_value>
class MapStd {
private:
	//std::unordered_map<T_key, T_value> all;
	std::map<T_key, T_value> all;
public:
	MapStd() = default;

	~MapStd() {
	  //delete_verbose("@Memory: MapStd deleted\n");
  }

	void insert(T_key key, T_value value) {
	  this->all.insert(std::pair<T_key, T_value>(key, value));
  }

	auto size () const {
	  return this->all.size();
  }

	T_value at (T_key key) {
	  return this->all.at(key);
  }

	bool contains (T_key key) {
    return all.count(key) > 0;
  };

	void update (T_key key, T_value value) {
    auto iter = this->all.find(key);
    if (iter == this->all.end())
    	this->all.insert(std::pair<T_key, T_value>(key, value));
    else
    	iter->second = value;
  }

	//std::string toString(std::string (*f_key) (T_key key), std::string (*f_value) (T_value value)) const;
	void clear () { all.clear(); }
	auto begin() -> auto {return all.begin();};
	auto end() -> auto {return all.end();};
};




template <typename T_value>
class MapArray {
private:
	T_value* all = nullptr;
	const unsigned int capacity{0};
public:

	MapArray(unsigned int capacity) : capacity(capacity) {
    if (capacity > 0) {
    	this->all = new T_value[capacity];
    	memset(all, 0, capacity * sizeof(T_value));
    }
  }

	~MapArray() {
	  delete[] this->all;
  }

	void insert(unsigned int key, T_value value) {
    assert(key < capacity && "OOB");
    all[key] = value;
  }

	unsigned int size () const { return capacity; }

	T_value at (unsigned int key) const {
    assert(key < capacity && "OOB");
    return all[key];
  }

	std::string toString(std::string (*f_value) (T_value value)) const {
    std::string s = "";
    for (unsigned int i = 0; i < this->capacity; i++){
    	s.append("\n\t\t");
    	// s.append(std::to_string(i));
    	s.append(std::to_string(this->all[i]->getId()));
    	s.append(" -> ");
    	s.append(f_value(this->all[i]));
    }
    return s;
  }

    class MapArrayIterator {
        unsigned int pos{0};
        MapArray *array;

    public:
        MapArrayIterator(MapArray* a) : array(a) {}
        MapArrayIterator(MapArray* a, unsigned pos) : pos(pos), array(a) {}

        MapArrayIterator& operator++() {
            ++pos;
            return *this;
        }

        MapArrayIterator operator++(int) {
            auto tmp = *this;
            operator++();
            return tmp;
        }

        T_value operator*() { return array->at(pos); }

        bool operator==(const MapArrayIterator& rhs) const {
            assert(array == rhs.array && "Comparing iterators for different arrays");
            return pos == rhs.pos;
        }

        bool operator!=(const MapArrayIterator& rhs) const { return !operator==(rhs); }
    };

    MapArrayIterator begin() { return MapArrayIterator(this); }
    MapArrayIterator end() { return MapArrayIterator(this, this->capacity); }
};




/*
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
*/


#endif /* QUAK_MAP_H_ */
