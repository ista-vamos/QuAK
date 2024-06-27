
#ifndef SET_H_
#define SET_H_

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <list>


template <typename T_element> class SetStd {
private:
	std::set<T_element> all;
	//std::unordered_set<T_element> all; // do not use unordered data structure without changing parser
public:
	SetStd();
	~SetStd();
	void insert(T_element element);
	void erase(T_element element);
	bool contains (T_element element) { return all.find(element) != all.end(); };
	unsigned int size() const;
	std::string toString (std::string (*f) (T_element element)) const;
	void clear () { all.clear(); };
	auto begin() {return all.begin();};
	auto end() {return all.end();};
	T_element getMin () { return *(all.begin());};
	T_element getMax () { return *(all.rbegin());};
};



template <typename T_element> class SetList {
private:
	std::list<T_element> all;
public:
	SetList();
	~SetList();
	void push(T_element element);
	void pop();
	T_element head();
	unsigned int size() const;
	auto begin() { return all.begin(); };
	auto end() { return all.end(); };
};



#include "Weight.h"
class SCC_Tree;
class Symbol;
class Word;
class ContextOf;
class StateRelation;
class TargetOf;
class State;
class Edge;

namespace std {
	template <> // Parser
	struct hash<std::pair<std::string, std::string>> {
		size_t operator()(const std::pair<std::string, std::string>& x) const {
			size_t h1 = hash<std::string>()(x.first);
			size_t h2 = hash<std::string>()(x.second);
			return h1 ^ h2;
		}
	};
	template <> // Parser
	struct hash<std::pair<std::string, weight_t>> {
		size_t operator()(const std::pair<std::string, weight_t>& x) const {
			size_t h1 = hash<std::string>()(x.first);
			size_t h2 = hash<weight_t>()(x.second);
			return h1 ^ h2;
		}
	};
	template <> // Parser
	struct hash<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>> {
		size_t operator()(const std::pair<std::pair<std::string, weight_t>, std::pair<std::string, std::string>>& x) const {
			size_t h1 = hash<std::pair<std::string, weight_t>>()(x.first);
			size_t h2 = hash<std::pair<std::string, std::string>>()(x.second);
			return h1 ^ h2;
		}
	};
	template <> // PostTargetVariable
	struct hash<std::pair<TargetOf*,Word*>> {
		size_t operator()(const std::pair<TargetOf*,Word*>& x) const {
			size_t h1 = hash<TargetOf*>()(x.first);
			size_t h2 = hash<Word*>()(x.second);
			return h1 ^ h2;
		}
	};
	template <>  // PostContextVariable
	struct hash<std::pair<Word*,weight_t>> {
		size_t operator()(const std::pair<Word*,weight_t>& x) const {
			size_t h1 = hash<Word*>()(x.first);
			size_t h2 = hash<weight_t>()(x.second);
			return h1 ^ h2;
		}
	};
	template <> // PostContextVariable
	struct hash<std::pair<ContextOf*,std::pair<Word*,weight_t>>> {
		size_t operator()(const std::pair<ContextOf*,std::pair<Word*,weight_t>>& x) const {
			size_t h1 = hash<ContextOf*>()(x.first);
			size_t h2 = hash<std::pair<Word*,weight_t>>()(x.second);
			return h1 ^ h2;
		}
	};
	template <> // Automaton
	struct hash<std::pair<State*, unsigned int>> {
		size_t operator()(const std::pair<State*, unsigned int>& x) const {
			size_t h1 = hash<State*>()(x.first);
			size_t h2 = hash<unsigned int>()(x.second);
			return h1 ^ h2;
		}
	};
	template <> // Automaton
	struct hash<std::pair<unsigned int, bool>> {
		size_t operator()(const std::pair<unsigned int, bool>& x) const {
			size_t h1 = hash<unsigned int>()(x.first);
			size_t h2 = hash<bool>()(x.second);
			return h1 ^ h2;
		}
	};
	template <> // Automaton
	struct hash<std::pair<State*,std::pair<unsigned int, bool>>> {
		size_t operator()(const std::pair<State*,std::pair<unsigned int, bool>>& x) const {
			size_t h1 = hash<State*>()(x.first);
			size_t h2 = hash<std::pair<unsigned int, bool>>()(x.second);
			return h1 ^ h2;
		}
	};
}

template class SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>>; // Parser
template class SetStd<std::string>; // Parser
template class SetStd<weight_t>; // Parser
template class SetStd<Symbol*>; // State
template class SetStd<State*>; // State
template class SetStd<Word*>;
template class SetStd<Edge*>;
template class SetStd<std::pair<TargetOf*,Word*>>;
template class SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>;
template class SetStd<std::pair<State*, unsigned int>>; // Automaton
template class SetStd<std::pair<State*,std::pair<unsigned int, bool>>>;
template class SetStd<std::pair<State*, Weight*>>;
template class SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>>;



template class SetList<SCC_Tree*>;
template class SetList<State*>;
template class SetList<Edge*>;

#endif /* SET_H_ */
