#include <iostream>
#include <memory>
#include <cstring>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

value_function_t getValueFunction(const char *str) {
#define CMP(S) ((strncmp(str, (S), sizeof(S))) == 0)
    if (CMP("Inf")) { return Inf; }
    if (CMP("Sup")) { return Sup; }
    if (CMP("LimInf")) { return LimInf; }
    if (CMP("LimSup")) { return LimSup; }
    if (CMP("LimAvg")) { return LimAvg; }
#undef CMP

    std::cerr << "Unknown value function: " << str << "\n";
    abort();
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
    auto A2 =  std::unique_ptr<Automaton>(new Automaton(argv[2]));

    auto value_fun = getValueFunction(argv[3]);

    if (booleanize) {
        std::cerr << "Not implemented yet\n";
        return -1;
    }

    bool included = A1->isIncludedIn(A2.get(), value_fun);

    std::cout << "Is included: " << included << "\n";

	return EXIT_SUCCESS;
}

