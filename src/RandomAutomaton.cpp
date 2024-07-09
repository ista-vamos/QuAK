#include <random>
#include <map>

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
  }

  auto alphabet_size = alphabet->size();
  assert(alphabet_size > 0);

  // create a copy of the alphabet
  MapArray<Symbol*>* new_alphabet = new MapArray<Symbol*>(alphabet_size);
  for (auto n = 0U; n < alphabet_size; ++n) {
    new_alphabet->insert(n, new Symbol(alphabet->at(n)));
  }

  // generate states
  MapArray<State*>* states = new MapArray<State*>(states_num); 
  for (auto n = 0U; n < states_num; ++n) {
    auto *state = new State(std::string("q") + std::to_string(n),
                            alphabet_size,
                            min_weight,
                            max_weight);
    states->insert(n, state);
  }

  // generate edges
  std::uniform_real_distribution<weight_t> rand_weight(min_weight, max_weight);
  std::uniform_int_distribution<unsigned> rand_state(0, states_num);
  std::uniform_int_distribution<unsigned> rand_symbol(0, alphabet_size);
  std::map<weight_t, Weight *> weights_register;

  while (edges_num > 0) {
    auto src_id = rand_state(reng);
    auto dest_id = rand_state(reng);
    auto weight = rand_weight(reng);
    auto symbol = rand_symbol(reng);

    State *src = states->at(src_id);
    State *dest = states->at(dest_id);

    auto *W = weights_register[weight];
    if (W == nullptr) {
      W = new Weight(weight);
      weights_register[weight] = W;
    }

		Edge *edge = new Edge(new_alphabet->at(symbol), W, src, dest);
		src->addSuccessor(edge);
		dest->addPredecessor(edge);

    --edges_num;
  }

  MapArray<Weight*>* weights = new MapArray<Weight*>(weights_register.size()); 
  unsigned n = 0;
  for (auto& it : weights_register) {
    weights->insert(n, it.second);
    ++n;
  }

  return new Automaton(name, new_alphabet, states, weights, min_weight, max_weight, states->at(0));
}
