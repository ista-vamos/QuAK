

#ifndef MAP_H_
#define MAP_H_

#include <iostream>
#include <string>
#include <map>


/*
template <typename T_key, typename T_value>
class Map {
protected:
	Map();
	~Map();
public:
	void insert(T_key key, T_value value);
	unsigned int size () const;
	T_value at (T_key key);
	//void operator() (Map<T_key, T_value> other);
	std::string toString(std::string (*f_key) (T_key key), std::string (*f_value) (T_value value)) const;

	////////
	//edges.insert(edges.begin() + a->getId(), q); // THIS IS FOR VECTOR
	//edges[a->getId()] = q; //THIS IS FOR ARRAY
	//edges.insert(pair<int, State*>(a->getId(), q)); // THIS IS FOR MAP
	////////
	//vector<State*> edges = {0}; //VECTOR DOES NOT WORK WELL FOR US
	//State* edges[100]; // ULTIMATELLY WE WILL USE ARRAY AFTER PARSING
	//map<int, State*> edges;
	////////
};

*/




template <typename T_key, typename T_value>
class MapStd { // : public Map<T_key, T_value> {
private:
	std::map<T_key, T_value> all;
public:
	MapStd();
	~MapStd();
	void insert(T_key key, T_value value);
	unsigned int size () const;
	T_value at (T_key key);
	std::string toString(std::string (*f_key) (T_key key), std::string (*f_value) (T_value value)) const;
	auto begin() {return all.begin();};
	auto end() {return all.end();};
	auto cbegin() {return all.cbegin();};
	auto cend() {return all.cend();};
};


template <typename T_value>
class MapVec {//: public Map<unsigned int, T_value> {
private:
	T_value* all = NULL;
	unsigned int capacity = 0;
public:
	//MapVec();
	~MapVec();
	MapVec(unsigned int capacity);
	void insert(unsigned int key, T_value value);
	unsigned int size () const;
	T_value at (unsigned int key) const;
	//void operator() (MapVec<T_value> other);
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



struct scc_data_struct;
class Symbol;
class State;
class Edge;
template <class T> class SetStd;

template class MapStd<std::string, Symbol*>;
template class MapStd<int, State*>;
template class MapStd<int, int>;
template class MapStd<std::string, State*>;

template class MapVec<scc_data_struct*>;
template class MapVec<int>;
template class MapVec<State*>;
template class MapVec<Symbol*>;
template class MapVec<SetStd<Edge*>*>;


#endif /* MAP_H_ */
