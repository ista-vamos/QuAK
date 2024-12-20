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
        auto A =  std::unique_ptr<Automaton>(
                Automaton::randomAutomaton("random", STATES_NUM,
                                           alphabet.get(),
                                           MIN_WEIGHT, MAX_WEIGHT,
                                           EDGES_NUM,
                                           /* complete=*/ true,
                                           STATES_NUM_IS_MAX
                                           )
        );

        bool included, included_bool;

        included = A->isIncludedIn(A.get(), VALUE_FUNCTION);
        included_bool = A->isIncludedIn(A.get(), VALUE_FUNCTION, /*booleanize=*/true);
        if (!included || !included_bool) {
          std::cerr << "Test FAILED!\n";
          std::cerr << "This " tostr(VALUE_FUNCTION) " automaton is not included in self:\n";
          std::cerr << "Result for antichains: " << included <<  " and for booleanized: " << included_bool << "\n";
          std::cerr << "---------\n";
          A->print();
          std::cerr << "---------\n";
          std::cerr << "This is the LimSup automaton from `toLimSup`:\n";
          std::cerr << "---------\n";
          auto *lsA = Automaton::toLimSup(A.get(), VALUE_FUNCTION);
          lsA->print();
          std::cerr << "---------\n";
          std::cerr << "Is " << lsA->getName() << " included in itself: " << lsA->isIncludedIn(lsA, LimSup);
          std::cerr << "\n---------\n";

          delete lsA;
          return EXIT_FAILURE;
        }
    }

	return EXIT_SUCCESS;
}

