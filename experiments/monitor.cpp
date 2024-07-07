#include <iostream>
#include <memory>
#include <cstring>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Monitor.h"
#include "Map.h"
#include "Word.h"

#include "utils.h"

static void monitorFile(Monitor *M, const char *input_file) {
    std::cerr << "Not implemented yet\n";
    abort();
}

static void monitorVamos(Monitor *M, const char *shmkey) {
    std::cerr << "Not implemented yet\n";
    abort();
}

int main(int argc, char **argv) {

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " automaton.txt " << "<Inf | Sup | LimInf | LimSup | LimAvg> [input-file | -vamos SHMKEY]\n";
        return -1;
    }

    auto A =  std::unique_ptr<Automaton>(new Automaton(argv[1]));
    auto value_fun = getValueFunction(argv[2]);

    auto M = std::unique_ptr<Monitor>(new Monitor(A.get(), value_fun));

    if (strncmp("-vamos", argv[3], 6) == 0) {
        if (argc != 5) {
            std::cerr << "Missing the SHMKEY argument\n";
            return -1;
        }

        monitorVamos(M.get(), argv[4]);
    } else {
        // read the input file
        monitorFile(M.get(), argv[3]);
    }

	return EXIT_SUCCESS;
}

