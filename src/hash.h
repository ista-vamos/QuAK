
#ifndef HASH_H_
#define HASH_H_



#include "Weight.h"
class SCC_Tree;
class Symbol;
class Word;
class ContextOf;
class StateRelation;
class TargetOf;
class State;
class Weight;
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
	template <>
	struct hash<std::pair<State*, Weight*>> {
		size_t operator()(const std::pair<State*, Weight*>& x) const {
			size_t h1 = hash<State*>()(x.first);
			size_t h2 = hash<Weight*>()(x.second);
			return h1 ^ h2;
		}
	};
	template <>
	struct hash<std::pair<std::pair<State*, Weight*>,std::pair<State*, Weight*>>> {
		size_t operator()(const std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>& x) const {
			size_t h1 = hash<std::pair<State*, Weight*>>()(x.first);
			size_t h2 = hash<std::pair<State*, Weight*>>()(x.second);
			return h1 ^ h2;
		}
	};
	template <>
	struct hash<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>,std::pair<State*, Weight*>>>> {
		size_t operator()(const std::pair<Symbol*, std::pair<std::pair<State*, Weight*>,std::pair<State*, Weight*>>>& x) const {
			size_t h1 = hash<Symbol*>()(x.first);
			size_t h2 = hash<std::pair<std::pair<State*, Weight*>,std::pair<State*, Weight*>>>()(x.second);
			return h1 ^ h2;
		}
	};
}



#endif /* HASH_H_ */
