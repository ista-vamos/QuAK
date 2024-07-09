#include <iostream>
#include <memory>
#include <cstring>
#include <ctime>
#include <cstdint>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Symbol.h"
#include "Map.h"
#include "Word.h"

constexpr unsigned REPETITIONS = 100;
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
                                           STATES_NUM_IS_MAX
                                           )
        );

        bool included;
        struct timespec start_time, end_time;

        included = A->isIncludedIn(A.get(), VALUE_FUNCTION);
        if (!included) {
          std::cerr << "Test FAILED!\n";
          std::cerr << "This " tostr(VALUE_FUNCTION) " automaton is not included in self:\n";
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

