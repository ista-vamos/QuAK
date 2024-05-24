
#include "Set.h"
#include "utility.h"

// -------------------------------- SetStd -------------------------------- //

template <typename T>
SetStd<T>::SetStd() {}

template <typename T>\
SetStd<T>::~SetStd() { delete_verbose("@Memory: SetStd deleted\n"); }


template <typename T>
void SetStd<T>::insert (T element) { this->all.insert(element); }


template <typename T>
void SetStd<T>::erase (T element) { this->all.erase(element); }


template <typename T>
unsigned int SetStd<T>::size () const { return this->all.size(); }


template <typename T>
std::string SetStd<T>::toString (std::string (*f) (T)) const {
	std::string s = "";

	for (T e : this->all){
		s.append("\n\t\t");
		s.append(f(e));
	}

	return s;
}


// -------------------------------- SetList -------------------------------- //


template <typename T_value>
SetList<T_value>::SetList() {}

template <typename T_value>
SetList<T_value>::~SetList() { delete_verbose("@Memory: SetList deleted\n"); }

template <typename T_value>
void SetList<T_value>::insert(T_value element) { all.push_front(element);  }

template <typename T_value>
unsigned int SetList<T_value>::size() const { return all.size(); }

template <typename T_value>
std::string SetList<T_value>::toString(std::string (*f) (T_value element)) const {
	fail("function not implemented");
	return "";
}



