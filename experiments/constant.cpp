#include <iostream>
#include <memory>
#include <cstring>
#include <ctime>
#include <cstdint>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

#include "utils.h"

#define CHECK_SAFETY_CLOSURE

static std::pair<unsigned, unsigned>
getAutomatonStats(const Automaton *A) {
    auto *states = A->getStates();
    unsigned n_states = states->size();
    unsigned n_edges = 0;

    auto *alphabet = A->getAlphabet();
    assert(alphabet);

    for (unsigned s = 0; s < n_states; ++s) {
        auto *state = states->at(s);
        assert(state);

        for (auto *symbol : *alphabet) {
            n_edges += state->getSuccessors(symbol->getId())->size();
        }
    }

    return {n_states, n_edges};
}

int main(int argc, char **argv) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " automaton1.txt <Inf | Sup | LimInf | LimSup | LimAvg>\n";
        std::cerr << "  Compute if `automaton1.txt` is constant assuming the given value function\n";
        return -1;
    }

    auto value_fun = getValueFunction(argv[2]);

    auto A =  std::unique_ptr<Automaton>(new Automaton(argv[1]));

    bool constant;
    struct timespec start_time, end_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    constant = A->isConstant(value_fun);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);

    end_time.tv_sec -= start_time.tv_sec;
    end_time.tv_nsec -= start_time.tv_nsec;

    unsigned n_states, n_edges;
    std::tie(n_states, n_edges) = getAutomatonStats(A.get());
    std::cout << "states/edges: " << n_states << "," << n_edges << "\n";
    std::cout << "Is constant: " << constant << "\n";
    std::cout << "Cputime: "
              << static_cast<uint64_t>((end_time.tv_sec * 1000000) +
                                       (end_time.tv_nsec / 1000.0))
              << " ms\n";

	return EXIT_SUCCESS;
}

