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


int main(int argc, char **argv) {

    auto alphabet = std::unique_ptr<MapArray<Symbol*>>(new MapArray<Symbol*>(ALPHABET_SIZE));
    for (auto n = 0U; n < ALPHABET_SIZE; ++n) {
      alphabet->insert(n, new Symbol(std::string(1, 'a' + n)));
    }

    for (auto n = 0U; n < REPETITIONS; ++n) {
        auto A =  std::unique_ptr<Automaton>(
                Automaton::randomAutomaton("random",
                                           STATES_NUM,
                                           alphabet.get(),
                                           MIN_WEIGHT,
                                           MAX_WEIGHT,
                                           EDGES_NUM,
                                           /* complete=*/ true,
                                           STATES_NUM_IS_MAX
                                           )
        );

        auto B = std::unique_ptr<Automaton>(
                Automaton::safetyClosure(A.get(), LimInfAvg));
        auto C = std::unique_ptr<Automaton>(Automaton::copy_trim_complete(B.get(), LimInfAvg));

        bool r1, r2;

        r1 = C->isConstant(LimInfAvg);
        r2 = C->isConstant(Inf);

        if (r1 != r2) {
          std::cerr << "Test FAILED!\n";
          std::cerr << "Safety closure of an automaton gives different `isConstant` for LimInfAvg and Inf:\n";
          std::cerr << r1 <<  " != " << r2 << "\n";
          std::cerr << "----- Automaton ----\n";
          A->print();
          std::cerr << "----- Its safety closure ----\n";
          B->print();
          std::cerr << "----- Trimmed and completed ----\n";
          C->print();
          return EXIT_FAILURE;
        }
    }

	return EXIT_SUCCESS;
}

