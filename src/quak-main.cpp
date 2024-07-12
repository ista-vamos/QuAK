#include <iostream>
#include <memory>
//#include <cstring>
//#include <ctime>
//#include <cstdint>
//#include <variant>

#include "Automaton.h"
#include "Map.h"

//#include "utils.h"
//
//static std::pair<unsigned, unsigned>
//getAutomatonStats(const Automaton *A) {
//    auto *states = A->getStates();
//    unsigned n_states = states->size();
//    unsigned n_edges = 0;
//
//    for (unsigned s = 0; s < n_states; ++s) {
//        auto *state = states->at(s);
//        assert(state);
//        auto *alphabet = state->getAlphabet();
//        assert(alphabet);
//
//        for (auto *symbol : *alphabet) {
//            n_edges += state->getSuccessors(symbol->getId())->size();
//        }
//    }
//
//    return {n_states, n_edges};
//}
//
//enum class Operation {
//  isNonempty,
//  isUniversal,
//  isIncluded,
//  isConstant,
//  isSafe,
//  isLive,
//  topValue,
//  bottomValue,
//};
//
//struct OperationClosure {
//  Operation op;
//  std::vector<std::variant<std::string, weight_t>> args;
//};
//
//
//struct Options {
//  std::vector<OperationClosure> actions;
//
//};

int main(int argc, char **argv) {

    if (argc < 4 || argc > 5) {
        std::cerr << "Usage: " << argv[0]
                  << " automaton.txt" << " [ACTION ACTION ...]\n";
        std::cerr << "Where ACTIONs are the following, with VALF = <Inf | Sup | LimInf | LimSup | LimAvg>:\n";
        std::cerr << "  empty VALF <weight>\n";
        std::cerr << "  non-empty VALF <weight>\n";
        std::cerr << "  universal VALF <weight>\n";
        std::cerr << "  constant VALF\n";
        std::cerr << "  safe VALF\n";
        std::cerr << "  live VALF\n";
        return -1;
    }

    bool booleanize = false;
    if (argc == 5) {
        if (strncmp("bool", argv[4], sizeof "bool") == 0) {
            booleanize = true;
        } else {
            std::cerr << "The optional argument is expected to be 'booleanize' or none\n";
            return -1;
        }
    }

    auto A1 =  std::unique_ptr<Automaton>(new Automaton(argv[1]));
    auto A2 =  std::unique_ptr<Automaton>(new Automaton(argv[2], A1.get()));


    //auto value_fun = getValueFunction(argv[3]);
    auto value_fun = LimSup;

    bool included;
//    struct timespec start_time, end_time;
//
//    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    included = A1->isIncludedIn(A2.get(), value_fun, booleanize);
//    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
//
//    end_time.tv_sec -= start_time.tv_sec;
//    end_time.tv_nsec -= start_time.tv_nsec;
//
//    unsigned n_states, n_edges;
//    std::tie(n_states, n_edges) = getAutomatonStats(A1.get());
//    std::cout << "A1 states/edges: " << n_states << "," << n_edges << "\n";
//    std::tie(n_states, n_edges) = getAutomatonStats(A2.get());
//    std::cout << "A2 states/edges: " << n_states << "," << n_edges << "\n";
    std::cout << "Is included: " << included << "\n";
//    std::cout << "Cputime: "
//              << static_cast<uint64_t>((end_time.tv_sec * 1000000) +
//                                       (end_time.tv_nsec / 1000.0))
//              << " ms\n";

	return EXIT_SUCCESS;
}




