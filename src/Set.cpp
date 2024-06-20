
#include "Set.h"
#include "utility.h"

// -------------------------------- SetStd -------------------------------- //

template <typename T_element>
SetStd<T_element>::SetStd() {}

template <typename T_element>
SetStd<T_element>::~SetStd() { delete_verbose("@Memory: SetStd deleted\n"); }


template <typename T_element>
void SetStd<T_element>::insert (T_element element) { this->all.insert(element); }


template <typename T_element>
void SetStd<T_element>::erase (T_element element) { this->all.erase(element); }


template <typename T_element>
unsigned int SetStd<T_element>::size () const {
	return this->all.size();
}


template <typename T_element>
std::string SetStd<T_element>::toString (std::string (*f) (T_element)) const {
	std::string s = "";
	for (T_element e : this->all){
		s.append("\n\t\t");
		s.append(f(e));
	}

	return s;
}


// -------------------------------- SetList -------------------------------- //



template <typename T_element>
SetList<T_element>::SetList() {}

template <typename T_element>
SetList<T_element>::~SetList() { delete_verbose("@Memory: SetList deleted\n"); }

template <typename T_element>
void SetList<T_element>::push(T_element element) { all.push_front(element); }

template <typename T_element>
void SetList<T_element>::queue(T_element element) { all.push_back(element); }

template <typename T_element>
T_element SetList<T_element>::head() { return *(all.begin()); }


template <typename T_element>
void SetList<T_element>::pop() { all.erase(all.begin()); }

template <typename T_element>
unsigned int SetList<T_element>::size() const { return all.size(); }

template <typename T_element>
std::string SetList<T_element>::toString (std::string (*f) (T_element element)) const {
	fail("function not implemented");
}



