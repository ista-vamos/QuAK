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

    if (argc < 4 || argc > 5) {
        std::cerr << "Usage: " << argv[0]
                  << " automaton1.txt" << " " << "automaton2.txt" << "<Inf | Sup | LimInf | LimSup | LimAvg> [booleanize]\n";
        std::cerr << "  Compute if `automaton1.txt` is included in `automaton2.txt` assuming the given value function\n";
        std::cerr << "  The optional argument 'booleanize' mean to use booleanized version of the inclusion\n";
        return -1;
    }

    bool booleanize = false;
    if (argc == 5) {
        if (strncmp("booleanize", argv[4], sizeof "booleanize") == 0) {
            booleanize = true;
        } else {
            std::cerr << "The optional argument is expected to be 'booleanize' or none\n";
            return -1;
        }
    }

    auto A1 =  std::unique_ptr<Automaton>(new Automaton(argv[1]));
    auto A2 =  std::unique_ptr<Automaton>(new Automaton(argv[2], A1.get()));

    auto value_fun = getValueFunction(argv[3]);

    bool included;
    struct timespec start_time, end_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    included = A1->isIncludedIn(A2.get(), value_fun, booleanize);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);

    end_time.tv_sec -= start_time.tv_sec;
    end_time.tv_nsec -= start_time.tv_nsec;

    unsigned n_states, n_edges;
    std::tie(n_states, n_edges) = getAutomatonStats(A1.get());
    std::cout << "A1 states/edges: " << n_states << "," << n_edges << "\n";
    std::tie(n_states, n_edges) = getAutomatonStats(A2.get());
    std::cout << "A2 states/edges: " << n_states << "," << n_edges << "\n";
    std::cout << "Is included: " << included << "\n";
    std::cout << "Cputime: "
              << static_cast<uint64_t>((end_time.tv_sec * 1000000) +
                                       (end_time.tv_nsec / 1000.0))
              << " ms\n";

	return EXIT_SUCCESS;
}

