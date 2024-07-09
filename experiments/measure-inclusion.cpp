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

constexpr unsigned REPEAT = 10U;
constexpr weight_t MIN_WEIGHT = -100.0;
constexpr weight_t MAX_WEIGHT =  100.0;
constexpr bool STATES_NUM_IS_MAX = true;

int main(int argc, char **argv) {

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " STATES_NUM ALPHABET_SIZE <Inf | Sup | LimInf | LimSup | LimAvg> AUTOMATA_NUM\n";
        return -1;
    }

    unsigned states_num = atoi(argv[1]);
    const unsigned alphabet_size = atoi(argv[2]);
    const unsigned automata_num = atoi(argv[4]);
    auto value_fun = getValueFunction(argv[3]);

    auto alphabet = std::unique_ptr<MapArray<Symbol*>>(new MapArray<Symbol*>(alphabet_size));
    for (auto n = 0U; n < alphabet_size; ++n) {
      alphabet->insert(n, new Symbol(std::string(1, 'a' + n)));
    }

    std::vector<std::unique_ptr<Automaton>> automata;
    automata.reserve(automata_num);
    for (auto n = 0U; n < automata_num; ++n) {
        auto A =  std::unique_ptr<Automaton>(
                Automaton::randomAutomaton("rnd-" + std::to_string(n),
                                           states_num,
                                           alphabet.get(),
                                           MIN_WEIGHT, MAX_WEIGHT,
                                           /* edges_num=*/0,
                                           /* complete=*/ true,
                                           STATES_NUM_IS_MAX
                                           )
        );
        automata.push_back(std::move(A));
    }

    bool included;
    struct timespec start_time, end_time;

    std::cout << "A1,A2,A1-size,A2-size,alphabet-size,cputime,included,booleanized\n";

    for (auto& A1 : automata) {
      for (auto& A2 : automata) {
        for (auto n = 0U; n < REPEAT; ++n) {
          for (bool booleanized : {true, false}) {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
            included = A1->isIncludedIn(A2.get(), value_fun, booleanized);
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);

            end_time.tv_sec -= start_time.tv_sec;
            end_time.tv_nsec -= start_time.tv_nsec;

            std::cout << A1->getName() << ","
                      << A2->getName() << ","
                      << A1->getStates()->size() << ","
                      << A2->getStates()->size() << ","
                      << alphabet_size << ","
                      << static_cast<uint64_t>((end_time.tv_sec * 1000000) +
                                               (end_time.tv_nsec / 1000.0)) << ","
                      << included << "," << booleanized << "\n";
          }
        }
      }
    }

	return EXIT_SUCCESS;
}

