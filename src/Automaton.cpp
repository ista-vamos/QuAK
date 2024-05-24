
#include <climits> // INT_MIN, INT_MAX
#include "Automaton.h"
#include "Parser.h"
#include "Edge.h"
#include "utility.h"


struct scc_data_struct {
	State* origin;
	int karp;
	int min_weight;
	int max_weight;
};



Automaton::~Automaton () {
	for (unsigned int id = 0; id < alphabet->size(); ++id) {
		delete alphabet->at(id);
	}
	for (unsigned int id = 0; id < states->size(); ++id) {
		delete states->at(id);
	}
	delete weights;
	if (SCCs != NULL) delete SCCs;
	delete_verbose("@Detail: 2 MapVec will be deleted (automaton)\n");
	delete_verbose("@Detail: 1 SetStd will be deleted (automaton)\n");
}

Automaton::Automaton (std::string filename) :
		name(filename),
		alphabet(NULL),
		states(NULL),
		SCCs(NULL),
		min_weight(INT_MAX),
		max_weight(INT_MIN),
		weights(NULL),
		initial (NULL)
{
	Parser parser(filename);
	MapStd<std::string, Symbol*> symbol_register;
	MapStd<std::string, State*> state_register;

	this->weights = new SetStd<int>();
	this->alphabet = new MapVec<Symbol*>(parser.alphabet.size());
	this->states = new MapVec<State*>(parser.states.size()+1);// +1 because of INITIAL

	for (std::string symbolname : parser.alphabet) {
		Symbol* symbol = new Symbol(symbolname);
		this->alphabet->insert(symbol->getId(), symbol);
		symbol_register.insert(symbol->getName(), symbol);
	}


	this->initial = new State(parser.initial, alphabet->size());
	this->states->insert(initial->getId(), initial);
	state_register.insert(initial->getName(), initial);
	for (std::string statename : parser.states) {
		State* state = new State(statename, alphabet->size());
		this->states->insert(state->getId(), state);
		state_register.insert(state->getName(), state);
	}

	for (auto tuple : parser.edges) {
		int weight = tuple.first.second;
		Symbol* symbol = symbol_register.at(tuple.first.first);
		State* from = state_register.at(tuple.second.first);
		State* to = state_register.at(tuple.second.second);
		weights->insert(weight);
		this->min_weight = std::min(this->min_weight, weight);
		this->max_weight = std::max(this->max_weight, weight);
		Edge *edge = new Edge(symbol, weight, from, to);
		from->addEdge(edge);
	}

	initialize_SCC();
	delete_verbose("@Detail: 2 MapStd will be deleted (register)\n");
}


State* Automaton::getInitial () const { return initial; }


bool Automaton::isDeterministic () const {
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			if (1 < this->states->at(state_id)->getSuccessors()->at(symbol_id)->size()) return false;
		}
	}
	return true;
}


unsigned int Automaton::initialize_SCC_recursive (State* state, int* time, int* discovery, SetList<State*>* list) const {
	unsigned int nb_SCCs = 0;
	discovery[state->getId()] = *time;
	state->setTag(*time);
	int min = *time;
	(*time)++;
	SetStd<Edge*>* set = state->getEdges();
	for (auto edge : *set) {
		if (discovery[edge->getTo()->getId()] == -1) {
			nb_SCCs += initialize_SCC_recursive(edge->getTo(), time, discovery, list);
		}
		min = std::min(min, edge->getTo()->getTag());
	}
	state->setTag(min);
	if (min==discovery[state->getId()]) {
		list->insert(state);
		return nb_SCCs+1;
	}
	return nb_SCCs;
}

void Automaton::initialize_SCC (void) {
	unsigned int size = this->states->size();
	int discovery[size];
	int time = 0;
	SetList<State*> list;
	for (unsigned int id = 0; id < size; ++id) {
		discovery[id] = -1;
	}
	unsigned int n = initialize_SCC_recursive(initial, &time, discovery, &list);
	this->SCCs = new MapVec<scc_data_struct*>(n);

	unsigned int id = 0;
	for (auto state : list) {
		scc_data_t* data = new scc_data_t;
		data->origin = state;
		this->SCCs->insert(id, data);
		++id;
	}

/*
	printf("THERE ARE %u SCCs\n", this->SCCs->size());
	for (unsigned int id = 0; id < this->SCCs->size(); ++id) {
		printf("SCC %d origin %s\n",
				SCCs->at(id)->origin->getTag(),
				SCCs->at(id)->origin->getName().c_str());
	}
*/
	return;
};


int Automaton::weight_reachably_recursive (State* state, bool scc_restriction, bool* discovery) const {
	if (discovery[state->getId()] == true) return min_weight - 1;
	discovery[state->getId()] = true;
	int weight = this->min_weight - 1;
	for (auto edge : *(state->getEdges())) {
		if (scc_restriction == false || edge->getTo()->getTag() == state->getTag()) {
			weight = std::max(weight, edge->getWeight());
			weight = std::max(weight, weight_reachably_recursive(edge->getTo(), scc_restriction, discovery));
		}
	}
	return weight;
}

int Automaton::weight_reachably (State* state, bool scc_restriction) const {
	unsigned int size = this->states->size();
	bool discovery[size];
	for (unsigned int id = 0; id < size; ++id) {
		discovery[id] = false;
	}
	return weight_reachably_recursive (state, scc_restriction, discovery);
}




int Automaton::weight_responce () const {
	int weight = this->min_weight - 1;
	for (unsigned int id = 0; id < this->SCCs->size(); ++id) {
		int n = weight_reachably(SCCs->at(id)->origin, true);
		weight = std::max(weight, n);
	}
	return weight;
}


int Automaton::weight_safety () const {
	fail("not implemented");
}
int Automaton::weight_persistance () const {
	fail("not implemented");
}


double Automaton::ONLYKarp (void) {
	unsigned int size = this->states->size();
	int distance[size+1][size];
	int infinity = size*max_weight+1;

	for (unsigned int len = 0; len <= size; ++len) {
		for (unsigned int id = 0; id < size; ++id) {
			distance[len][id] = infinity;
		}
	}

	for (unsigned int scc_id = 0; scc_id < this->SCCs->size(); ++scc_id) {
		distance[0][this->SCCs->at(scc_id)->origin->getId()] = 0;
	}

	for (unsigned int len = 1; len <= size; ++len) {
		for (unsigned int id = 0; id < size; ++id)	{
			for (auto edge : *(states->at(id)->getEdges())) {
				if (edge->getFrom()->getTag() == edge->getTo()->getTag()) {
					if (distance[len-1][edge->getFrom()->getId()] != infinity) {
						int x = distance[len-1][edge->getFrom()->getId()] - edge->getWeight();
						if (distance[len][edge->getTo()->getId()] == infinity) {
							distance[len][edge->getTo()->getId()] = x;
						}
						else {
							distance[len][edge->getTo()->getId()] = std::min(x,
									distance[len][edge->getTo()->getId()]
							);
						}
					}
				}
			}
		}
	}


	double min_id = max_weight + 0.0;
	bool id_flag = false;
	for (unsigned int id = 0; id < size; ++id) {
		double max_len = min_weight + 0.0;
		bool len_flag = false;
		if (distance[size][id] != infinity) {
			for (unsigned int len = 0; len < size; ++len) {
				if (distance[len][id] != infinity) {
					double x = (distance[size][id] - distance[len][id] + 0.0) / (size - len + 0.0);
					max_len = std::max(max_len, x);
					len_flag = true;
				}
			}
		}
		if (len_flag) {
			min_id = std::min(min_id, max_len);
			id_flag = true;
		}
	}
	if (id_flag == false) min_id = min_weight - 1;

	return min_id;
}



void Automaton::emptiness() {
	//int (*max)(int, int) = [](int a, int b) { return std::max(a, b); };
	printf("\temptiness:\n");
	int x = weight_reachably(initial, false);
	printf("\t\t   Sup: exists w : A(w) > x iff %s > x\n", x<min_weight ? "-infinity" : std::to_string(x).c_str());
	double y = ONLYKarp();
	printf("\t\tLimAvg: exists w : A(w) > y iff %s > y\n", y<min_weight ? "-infinity" : std::to_string(y).c_str());
	int z = weight_responce();
	printf("\t\tLimSup: exists w : A(w) > z iff %s > z\n", z<min_weight ? "-infinity" : std::to_string(z).c_str());
}


std::string Automaton::toString () const {
	std::string s = "Automaton \"";
	s.append(this->name);
	s.append("\":\n\t");
	s.append("alphabet:");
	s.append(alphabet->toString(Symbol::toString));
	s.append("\n\t");
	s.append("weights:");
	s.append(weights->toString(std::to_string));
	s.append("\n\t\tMIN = ");
	s.append(std::to_string(min_weight));
	s.append("\n\t\tMAX = ");
	s.append(std::to_string(max_weight));
	s.append("\n\t");
	s.append("states:");
	s.append(states->toString(State::toString));
	s.append("\n\t\tINITIAL = ");
	s.append(initial->toString());
	s.append("\n\t");
	s.append("edges:");
	for (unsigned int id = 0; id < states->size(); ++id) {
		s.append(states->at(id)->getEdges()->toString(Edge::toString));
	}
	return s;
}







/*
void Automaton::Karp (void) {
	unsigned int size = this->states->size();
	int distance[size+1][size];
	int infinity = size*max_weight+1;

	for (unsigned int len = 0; len <= size; ++len) {
		for (unsigned int id = 0; id < size; ++id) {
			distance[len][id] = infinity;
		}
	}
	distance[0][0] = 0;

	for (unsigned int len = 1; len <= size; ++len) {
		for (unsigned int id = 0; id < size; ++id)	{
			SetStd<Edge*>* set = states->at(id)->getEdges();
			for (auto iter = set->cbegin(); iter != set->cend(); ++iter) {
				Edge* edge = *iter;
				if (distance[len-1][edge->getFrom()->getId()] != infinity) {
					int x = distance[len-1][edge->getFrom()->getId()] + edge->getWeight();
					if (distance[len][edge->getTo()->getId()] == infinity) {
						distance[len][edge->getTo()->getId()] = x;
					}
					else {
						distance[len][edge->getTo()->getId()] = std::min(x,
								distance[len][edge->getTo()->getId()]
						);
					}
				}
			}
		}
	}

	double min_id = max_weight + 0.0;
	for (unsigned int id = 0; id < size; ++id) {
		double max_len = min_weight + 0.0;
		bool flag = false;
		if (distance[size][id] != infinity) {
			for (unsigned int len = 0; len < size; ++len) {
				if (distance[len][id] != infinity) {
					double x = (distance[size][id] - distance[len][id] + 0.0) / (size - len + 0.0);
					max_len = std::max(max_len, x);
					flag = true;
				}
			}
		}
		if (flag) min_id = std::min(min_id, max_len);
	}

	printf("KARP : %lf\n", min_id);
}
*/



/*void shortest (State* source) const {
	printf("################################# SHORTEST PATH BEGIN\n");fflush(stdout);
	unsigned int length = this->states.size();
	printf("LENGTH = %u\n", length);fflush(stdout);
	unsigned int* edge_distance = new unsigned int[length];
	unsigned int* weight_distance = new unsigned int[length];
	for (unsigned int key = 0; key < length; ++key) {
		edge_distance[states.at(key)->getId()] = UINT_MAX;
		weight_distance[states.at(key)->getId()] = UINT_MAX;
	}
	edge_distance[source->getId()] = 0;
	weight_distance[source->getId()] = 0;


	std::multimap<int, State*> queue = std::multimap<int, State*>();
	queue.insert(std::pair<int, State*>(0,source));
	// (Map: distance -> state)

	bool* inQueue = new bool[length];
	memset(inQueue, false, length*sizeof(bool));
	inQueue[source->getId()] = true;

	while (queue.size() > 0) {
		//int x = queue.begin()->first;
		State* from = queue.begin()->second;
		queue.erase(queue.begin());
		inQueue[from->getId()] = false;
		printf("State %s pop from the queue distance (%u)\n",
				from->getName().c_str(),
				weight_distance[from->getId()]-min_weight*edge_distance[from->getId()]
		);fflush(stdout);

		SetStd<Edge*>* set = from->getEdges();/
		for (auto iter = set->cbegin(); iter != set->cend(); ++iter) {
			printf("Edge (%s) considered\n", (*iter)->toString().c_str());fflush(stdout);
			State* to = (*iter)->getTo();
			int weight = (*iter)->getWeight();
			printf("State %s -> %s old distance (%u - %d -> %u)\n",
					from->getName().c_str(),
					to->getName().c_str(),
					distance[from->getId()],
					weight + min_weight,
					distance[to->getId()]
			);fflush(stdout);
			if (distance[to->getId()] > distance[from->getId()] + weight + min_weight) {
				printf("State %s new distance (%u)\n",
						to->getName().c_str(),
						distance[to->getId()] + weight + min_weight
				);fflush(stdout);
				distance[to->getId()] = distance[from->getId()] + weight + min_weight;
				if (inQueue[to->getId()] == false) {
					printf("State %s in queue\n", to->getName().c_str());fflush(stdout);
					queue.insert(std::pair<int, State*>(distance[to->getId()],to));
					inQueue[to->getId()] = true;
				}
			}
		}
	}

	for (unsigned int key = 0; key < length; ++key) {
		State* state = states.at(key);
		printf("DISTANCE FROM INITIAL %s = %u\n", state->getName().c_str(), distance[state->getId()]-min_weight);fflush(stdout);
	}
	delete[] distance;
	delete[] inQueue;
	printf("################################# SHORTEST PATH END\n");fflush(stdout);
}
*/


