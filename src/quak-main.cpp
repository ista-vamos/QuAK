#include <iostream>
#include <memory>
#include <cstring>
#include <variant>
#include <ctime>

#include "Automaton.h"
#include "Map.h"
#include "Monitor.h"

#include "utils.h"

void monEvalTrace(Monitor *M, const std::string& trace);

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

enum class Operation {
  INVALID,
  stats,
  dump,
  isEmpty,
  isNonempty,
  isUniversal,
  isIncluded,
  isIncludedBool,
  isConstant,
  isSafe,
  isLive,
  topValue,
  bottomValue,
  eval,
  monitor
};

static void printUsage(const char *bin) {
  std::cerr << "Usage: " << bin
            << " [-cputime] [-v] [-d] automaton-file" << " [ACTION ACTION ...]\n";
  std::cerr << "Where ACTIONs are the following, with VALF = <Inf | Sup | LimInf | LimSup | LimAvg>:\n";
  std::cerr << "  stats\n";
  std::cerr << "  dump\n";
  std::cerr << "  empty VALF <weight>\n";
  std::cerr << "  non-empty VALF <weight>\n";
  std::cerr << "  universal VALF <weight>\n";
  std::cerr << "  constant VALF\n";
  std::cerr << "  safe VALF\n";
  std::cerr << "  live VALF\n";
  std::cerr << "  isIncluded VALF automaton2-file\n";
  std::cerr << "  isIncludedBool VALF automaton2-file\n";
  std::cerr << "  eval <Inf | Sup | Avg> word-file\n";
  std::cerr << "  monitor <Inf | Sup | Avg> word-file\n";
  std::cerr << "  monitor-vamos <Inf | Sup | Avg> shmkey\n";
}

struct OperationClosure {
  Operation op{Operation::INVALID};
  std::vector<std::variant<std::string, weight_t, value_function_t>> args;
};


struct Options {
  std::string automaton;
  std::vector<OperationClosure> actions;
  std::string error;
  bool cputime{false};
  bool verbose{false};
  bool dump{false};

  static Options createError(const std::string& err) {
    Options O;
    O.error = err;
    return O;
  }
};

static weight_t getWeight(const char *str, bool& succ) {
  char *endptr;
  static_assert(std::is_same<float, weight_t::T>::value,
                "Weights are not floats, fix the code");
  double val = strtof(str, &endptr);

  succ = endptr != str && *endptr == '\0';
  return val;
}


Options parseArgs(int argc, char *argv[]) {
#define streq(s, cs) (strncmp(s, cs, sizeof(cs)) == 0)

  int idx = 1;
  if (argc < 2) {
    return Options::createError("No automaton given");
  }

  Options O;

  while (idx < argc) {
    if (streq(argv[idx], "-cputime"))
      O.cputime = true;
    else if (streq(argv[idx], "-v"))
      O.verbose = true;
    else if (streq(argv[idx], "-d"))
      O.dump = true;
    else if (argv[idx][0] != '-')
      break;

    ++idx;
  }

  if (idx < argc) {
    O.automaton = std::string(argv[idx++]);
  } else {
    return Options::createError("Invalid arguments, expected automaton file.");
  }

  while (idx < argc) {
    OperationClosure cl;

    if (streq(argv[idx], "stats")) {
      cl.op = Operation::stats;
    } else if (streq(argv[idx], "dump")) {
      cl.op = Operation::dump;
    } else if (streq(argv[idx], "empty")) {
      cl.op = Operation::isEmpty;
    } else if (streq(argv[idx], "non-empty")) {
      cl.op = Operation::isNonempty;
    } else if (streq(argv[idx], "universal")) {
      cl.op = Operation::isUniversal;
    } else if (streq(argv[idx], "constant")) {
      cl.op = Operation::isConstant;
    } else if (streq(argv[idx], "safe")) {
      cl.op = Operation::isSafe;
    } else if (streq(argv[idx], "live")) {
      cl.op = Operation::isLive;
    } else if (streq(argv[idx], "isIncluded")) {
      cl.op = Operation::isIncluded;
    } else if (streq(argv[idx], "isIncludedBool")) {
      cl.op = Operation::isIncludedBool;
    } else if (streq(argv[idx], "monitor")) {
      cl.op = Operation::monitor;
    }


    if (cl.op == Operation::stats || cl.op == Operation::dump) {
         O.actions.push_back(cl);
         ++idx;
    } else if (cl.op == Operation::isNonempty ||
        cl.op == Operation::isEmpty ||
        cl.op == Operation::isUniversal) {
      if (idx + 2 >= argc) {
        return Options::createError("Invalid arguments for " + std::string(argv[idx]));
      }

      cl.args.push_back(getValueFunction(argv[idx + 1]));

      bool succ;
      weight_t w = getWeight(argv[idx + 2], succ);
      if (!succ) {
        return Options::createError("Invalid weight: " + std::string(argv[idx + 2]));
      }
      cl.args.push_back(w);
      O.actions.push_back(cl);

      idx += 3;
    } else if (cl.op == Operation::isConstant ||
               cl.op == Operation::isSafe ||
               cl.op == Operation::isLive) {
      if (idx + 1 >= argc) {
        return Options::createError("Invalid arguments for " + std::string(argv[idx]));
      }

      cl.args.push_back(getValueFunction(argv[idx + 1]));
      O.actions.push_back(cl);

      idx += 2;
    } else if (cl.op == Operation::isIncluded ||
               cl.op == Operation::isIncludedBool) {
      if (idx + 2 >= argc) {
        return Options::createError("Invalid arguments for " + std::string(argv[idx]));
      }

      cl.args.push_back(getValueFunction(argv[idx + 1]));
      cl.args.push_back(std::string(argv[idx + 2]));
      O.actions.push_back(cl);

      idx += 3;
    } else if (cl.op == Operation::monitor) {
      if (idx + 2 >= argc) {
        return Options::createError("Invalid arguments for " + std::string(argv[idx]));
      }

      cl.args.push_back(getValueFunction(argv[idx + 1]));
      cl.args.push_back(std::string(argv[idx + 2]));
      O.actions.push_back(cl);

      idx += 3;
    }

    else {
      return Options::createError("Unknown action: " + std::string(argv[idx]));
    }
  }

#undef streq
  return O;
}

#define TIMER_INIT struct timespec start_time{0, 0}, end_time{0, 0};
#define TIMER_START if (opts.cputime) { clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time); }
#define TIMER_END if (opts.cputime) { clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time); }
#define TIMER_GET static_cast<uint64_t>((end_time.tv_sec * 1000) + (end_time.tv_nsec / 1000000.0))
#define TIMER_PRINT(msg) if (opts.cputime) { std::cout << msg << TIMER_GET << " ms\n"; }
#define PRINT_DIV { std::cout << "----------\n"; }

int main(int argc, char **argv) {

    auto opts = parseArgs(argc, argv);
    if (!opts.error.empty()) {
      std::cerr << "ERROR: " << opts.error << "\n";
      printUsage(argv[0]);
      return -1;
    }   
    if (opts.actions.empty()) {
      std::cerr << "No actions given\n";
      printUsage(argv[0]);
      return -1;
    }   

    TIMER_INIT

    TIMER_START
    auto A =  std::unique_ptr<Automaton>(new Automaton(opts.automaton));
    TIMER_END
    TIMER_PRINT("Cputime of building the automaton: ")

    if (opts.verbose) {
      unsigned n_states, n_edges;
      std::tie(n_states, n_edges) = getAutomatonStats(A.get());
      std::cout << "Input automaton has " << n_states
                << " states and " << n_edges << " edges.\n";
    }

    if (opts.dump) {
      A->print();
    }

    PRINT_DIV

    value_function_t value_fun;
    weight_t weight;


    for (auto& act : opts.actions) {
      switch (act.op) {
      case Operation::stats:
        {
          unsigned n_states, n_edges;
          std::tie(n_states, n_edges) = getAutomatonStats(A.get());
          std::cout << "Input automaton has " << n_states
                    << " states and " << n_edges << " edges.\n";
          PRINT_DIV
        }
        break;
      case Operation::dump:
        {
           A->print();
          PRINT_DIV
        }
        break;
      case Operation::isEmpty:
        value_fun = std::get<value_function_t>(act.args[0]);
        weight = std::get<weight_t>(act.args[1]);
        std::cout << "isEmpty("
                  << valueFunctionToStr(value_fun)
                  << ", weight=" << weight << ") = ";
        {
        TIMER_START
        auto r = !A->isNonEmpty(value_fun, weight);
        TIMER_END
        std::cout << r << "\n";
        TIMER_PRINT("Cputime: ")
        PRINT_DIV
        }
        break;

      case Operation::isNonempty:
        value_fun = std::get<value_function_t>(act.args[0]);
        weight = std::get<weight_t>(act.args[1]);
        std::cout << "isNonEmpty("
                  << valueFunctionToStr(value_fun)
                  << ", weight=" << weight << ") = ";
        {
        TIMER_START
        auto r = A->isNonEmpty(value_fun, weight);
        TIMER_END
        std::cout << r << "\n";
        TIMER_PRINT("Cputime: ")
        PRINT_DIV
        }
        break;

      case Operation::isUniversal:
        value_fun = std::get<value_function_t>(act.args[0]);
        weight = std::get<weight_t>(act.args[1]);
        std::cout << "isUniversal("
                  << valueFunctionToStr(value_fun)
                  << ", weight=" << weight << ") = ";
        {
        TIMER_START
        auto r = A->isUniversal(value_fun, weight);
        TIMER_END
        std::cout << r << "\n";
        TIMER_PRINT("Cputime: ")
        PRINT_DIV
        }

        break;

      case Operation::isConstant:
        value_fun = std::get<value_function_t>(act.args[0]);
        std::cout << "isConstant("
                  << valueFunctionToStr(value_fun)
                  << ") = ";
        {
        TIMER_START
        auto r = A->isConstant(value_fun);
        TIMER_END
        std::cout << r << "\n";
        TIMER_PRINT("Cputime: ")
        PRINT_DIV
        }
        break;

      case Operation::isSafe:
        value_fun = std::get<value_function_t>(act.args[0]);
        std::cout << "isSafe("
                  << valueFunctionToStr(value_fun)
                  << ") = ";
        {
        TIMER_START
        auto r = A->isSafe(value_fun);
        TIMER_END
        std::cout << r << "\n";
        TIMER_PRINT("Cputime: ")
        PRINT_DIV
        }

        break;

      case Operation::isLive:
        value_fun = std::get<value_function_t>(act.args[0]);
        std::cout << "isLive("
                  << valueFunctionToStr(value_fun)
                  << ") = ";
        {
        TIMER_START
        auto r = A->isLive(value_fun);
        TIMER_END
        std::cout << r << "\n";
        TIMER_PRINT("Cputime: ")
        PRINT_DIV
        }
        break;

      case Operation::isIncluded:
      case Operation::isIncludedBool:
        {
        TIMER_START
        auto B = std::unique_ptr<Automaton>(
            new Automaton(std::get<std::string>(act.args[1]), A.get()));
        TIMER_END
        TIMER_PRINT("Cputime of building the right-hand side automaton: ")

        if (opts.dump) {
          B->print();
        }

        TIMER_START
        value_fun = std::get<value_function_t>(act.args[0]);
        std::cout << "isIncluded(";
        if (act.op == Operation::isIncludedBool)
          std::cout << "bool, ";
        std::cout << valueFunctionToStr(value_fun)
                  << ") = ";

        TIMER_START
        auto r =  A->isIncludedIn(B.get(), value_fun,
                                  act.op == Operation::isIncludedBool);
        TIMER_END
        std::cout << r << "\n";
        TIMER_PRINT("Cputime: ")
        PRINT_DIV
        }
        break;

      case Operation::monitor:
        value_fun = std::get<value_function_t>(act.args[0]);
        std::cout << "monitor("
                  << valueFunctionToStr(value_fun)
                  << ") =\n";

        {
          auto M = std::unique_ptr<Monitor>(
            new Monitor(A.get(), value_fun)
          );

          auto trace = std::get<std::string>(act.args[1]);
          TIMER_START
          monEvalTrace(M.get(), trace);
          TIMER_END
          TIMER_PRINT("Cputime (incl. prints): ")
          PRINT_DIV
        }
        break;
      default:
        std::cerr << "Unknown operation\n";
        abort();
      }
    }
    

    /*
    auto A2 =  std::unique_ptr<Automaton>(new Automaton(argv[2]));

    bool included;

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
  */

	return EXIT_SUCCESS;
}



void monEvalTrace(Monitor *M, const std::string& trace) {
  std::ifstream stream(trace);
  if (!stream.is_open()) {
    std::cerr << "Failed opening file: " << trace << "\n";
    abort();
  }

  std::string symbol;
  while (stream) {
    stream >> symbol;
    std::cout << symbol << " -> " << M->next(symbol) << "\n" << std::flush;

  }

}

