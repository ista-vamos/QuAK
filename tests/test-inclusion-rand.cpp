#include <iostream>
#include <memory>
#include <cstring>
#include <cstdint>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Symbol.h"
#include "Map.h"
#include "Word.h"

constexpr bool STATES_NUM_IS_MAX=false;

#define tostr(x) _tostr(x)
#define _tostr(x) #x

int main(int argc, char **argv) {

    auto alphabet = std::unique_ptr<MapArray<Symbol*>>(new MapArray<Symbol*>(ALPHABET_SIZE));
    for (auto n = 0U; n < ALPHABET_SIZE; ++n) {
      alphabet->insert(n, new Symbol(std::string(1, 'a' + n)));
    }

    for (auto n = 0U; n < REPETITIONS; ++n) {
        auto A1 =  std::unique_ptr<Automaton>(
                Automaton::randomAutomaton("A1", STATES_NUM,
                                           alphabet.get(),
                                           MIN_WEIGHT, MAX_WEIGHT,
                                           EDGES_NUM,
                                           /* complete=*/ true,
                                           STATES_NUM_IS_MAX
                                           )
        );

        auto A2 =  std::unique_ptr<Automaton>(
                Automaton::randomAutomaton("A2", STATES_NUM,
                                           alphabet.get(),
                                           MIN_WEIGHT, MAX_WEIGHT,
                                           EDGES_NUM,
                                           /* complete=*/ true,
                                           STATES_NUM_IS_MAX
                                           )
        );

        bool included, included_bool;

        included = A1->isIncludedIn(A2.get(), VALUE_FUNCTION);
        included_bool = A1->isIncludedIn(A2.get(), VALUE_FUNCTION, /*booleanize=*/true);
        if (included != included_bool) {
          std::cerr << "Test FAILED!\n";
          std::cerr << "Inclusion for these " tostr(VALUE_FUNCTION) " automata gives different results with and without booleanization:\n";
          std::cerr << "Result for antichains: " << included <<  " and for booleanized: " << included_bool << "\n";
          std::cerr << "---------\n";
          A1->print(/*full=*/true, /* bv_weights =*/ true);
          std::cerr << "---------\n";
          A2->print(/*full=*/true, /* bv_weights =*/ true );
          std::cerr << "---------\n";
          return EXIT_FAILURE;
        }
    }

	return EXIT_SUCCESS;
}

