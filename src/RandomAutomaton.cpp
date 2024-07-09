#include <random>
#include <map>
#include <vector>

#include "Automaton.h"
#include "State.h"
#include "Edge.h"
#include "Weight.h"

Automaton *Automaton::randomAutomaton(const std::string& name,
                                      unsigned states_num,
                                      MapArray<Symbol*>* alphabet,
                                      weight_t min_weight,
                                      weight_t max_weight,
                                      // number of edges, if set to `0`, the number is
                                      // going to be a random number between `states_num / 2` and `states_num*states_num`
                                      unsigned edges_num,
                                      // should we generate exactly `states_num` states
                                      // or _at most_ `states_num` states?
                                      bool states_num_is_max) {

  Weight::RESET();
  State::RESET();
	Symbol::RESET(alphabet->size());

  std::random_device r;
  std::default_random_engine reng(r());
 
  if (states_num_is_max) {
    std::uniform_int_distribution<unsigned> rand(1, states_num);
    states_num = rand(reng);
  }

  if (edges_num == 0) {
    std::uniform_int_distribution<unsigned> rand(states_num / 2.0, states_num*states_num);
    edges_num = rand(reng);
    if (edges_num == 0) {
      edges_num = states_num;
    }
  }

  auto alphabet_size = alphabet->size();
  assert(alphabet_size > 0);

  // create a copy of the alphabet
  MapArray<Symbol*>* new_alphabet = new MapArray<Symbol*>(alphabet_size);
  for (auto n = 0U; n < alphabet_size; ++n) {
    new_alphabet->insert(n, new Symbol(alphabet->at(n)));
  }

  // generate edges
  std::uniform_real_distribution<weight_t> rand_weight(min_weight, max_weight);
  std::uniform_int_distribution<unsigned> rand_state(0, states_num - 1);
  std::uniform_int_distribution<unsigned> rand_symbol(0, alphabet_size - 1);
  std::map<weight_t, Weight *> weights_register;

  weight_t real_min = max_weight;
  weight_t real_max = min_weight;

  std::vector<std::tuple<unsigned, unsigned, weight_t, unsigned>> tmpedges;
  tmpedges.reserve(edges_num);

  while (edges_num > 0) {
    auto src_id = rand_state(reng);
    auto dest_id = rand_state(reng);
    auto weight = rand_weight(reng);
    auto symbol = rand_symbol(reng);

    auto *W = weights_register[weight];
    if (W == nullptr) {
      W = new Weight(weight);
      weights_register[weight] = W;
    }

    if (weight < real_min)
      real_min = weight;
    if (weight > real_max)
      real_max = weight;

    tmpedges.push_back({src_id, dest_id, weight, symbol});

    --edges_num;
  }

  ///////////////////////////////////////////////////////////////////
  // Generate the automaton
  ///////////////////////////////////////////////////////////////////
  
  // generate weights
  MapArray<Weight*>* weights = new MapArray<Weight*>(weights_register.size()); 
  unsigned n = 0;
  for (auto& it : weights_register) {
    weights->insert(n, it.second);
    ++n;
  }

  // generate states
  MapArray<State*>* states = new MapArray<State*>(states_num); 
  for (auto n = 0U; n < states_num; ++n) {
    auto *state = new State(std::string("q") + std::to_string(n),
                            alphabet_size,
                            real_min,
                            real_max);
    states->insert(n, state);
  }

  // generate edges
  for (auto& tmpedge : tmpedges) {
    State *src = states->at(std::get<0>(tmpedge));
    State *dest = states->at(std::get<1>(tmpedge));
    Weight *W = weights_register[std::get<2>(tmpedge)];
    assert(W != nullptr);

    Edge *edge = new Edge(new_alphabet->at(std::get<3>(tmpedge)), W, src, dest);
    src->addSuccessor(edge);
    dest->addPredecessor(edge);
  }

  return new Automaton(name, new_alphabet, states, weights, real_min, real_max, states->at(0));
}
