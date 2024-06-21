
#include <cstring> //memset
#include "Map.h"
#include "utility.h"


// -------------------------------- MapStd -------------------------------- //


template <typename T_key, typename T_value>
MapStd<T_key, T_value>::MapStd() {}


template <typename T_key, typename T_value>
MapStd<T_key, T_value>::~MapStd() {
	delete_verbose("@Memory: MapStd deleted\n");
}


template <typename T_key, typename T_value>
void MapStd<T_key, T_value>::insert (T_key key, T_value value) {
	this->all.insert(std::pair<T_key, T_value>(key, value));
}

template <typename T_key, typename T_value>
void MapStd<T_key, T_value>::update (T_key key, T_value value) {
	auto iter = this->all.find(key);
	if (iter == this->all.end())
		this->all.insert(std::pair<T_key, T_value>(key, value));
	else
		iter->second = value;
}



template <typename T_key, typename T_value>
T_value MapStd<T_key, T_value>::at (T_key key) {
	return this->all.at(key);
}



template <typename T_key, typename T_value>
unsigned int MapStd<T_key, T_value>::size () const {
	return this->all.size();
}



template <typename T_key, typename T_value>
std::string MapStd<T_key, T_value>::toString (std::string (*f_key) (T_key key), std::string (*f_value) (T_value value)) const {
	std::string s = "";
	for (std::pair<T_key, T_value> pair : this->all){
		s.append("\n\t\t");
		s.append(f_value(pair.second));
		s.append(" -> ");
		s.append(f_key(pair.first));
	}
	return s;
}




// -------------------------------- MapArray -------------------------------- //


template <typename T_value>
MapArray<T_value>::MapArray(unsigned int capacity) {
	if (capacity > 0) {
		this->all = new T_value[capacity];
		memset(all, 0, capacity * sizeof(T_value));
	}
	this->capacity = capacity;
}


template <typename T_value>
MapArray<T_value>::~MapArray() {
	delete_verbose("@Memory: MapVec deleted (array size = %d)\n", this->capacity);
	delete[] this->all;
}


template <typename T_value>
void MapArray<T_value>::insert(unsigned int key, T_value value) {
	if (key < this->capacity) {
		all[key] = value;
	}
	else {
		fail("MapVec insert failure");//fixme : remove
	}
}


template <typename T_value>
unsigned int MapArray<T_value>::size () const {
	return this->capacity;
}


template <typename T_value>
std::string MapArray<T_value>::toString (std::string (*f_value) (T_value value)) const {
	std::string s = "";
	for (unsigned int i = 0; i < this->capacity; i++){
		s.append("\n\t\t");
		s.append(std::to_string(i));
		s.append(" -> ");
		s.append(f_value(this->all[i]));
	}
	return s;
}


template <typename T_value>
T_value MapArray<T_value>::at (unsigned int key) const {
	if (key < this->capacity) {
		return all[key];
	}
	else {
		fail("MapVec at failure");//fixme : remove
	}
}



