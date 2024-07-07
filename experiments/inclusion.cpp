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

    std::cerr << "Unknown value function: " << str << "\n";
    abort();
}

int main(int argc, char **argv) {

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " automaton1.txt" << " " << "automaton2.txt" << "<Inf | Sup | LimInf | LimSup | LimAvg>\n";
        std::cerr << "  Compute if `automaton1.txt` is included in `automaton2.txt` assuming the given value function\n";
        return -1;
    }

    auto A1 =  std::unique_ptr<Automaton>(new Automaton(argv[1]));
    auto A2 =  std::unique_ptr<Automaton>(new Automaton(argv[2]));

    auto value_fun = getValueFunction(argv[3]);

    bool included = A1->isIncludedIn(A2.get(), value_fun);

    std::cout << "Is included: " << included << "\n";

	return EXIT_SUCCESS;
}

