#include <iostream>
#include <cstring>

#include "Automaton.h"

value_function_t getValueFunction(const char *str) {
#define CMP(S) ((strncmp(str, (S), sizeof(S))) == 0)
    if (CMP("Inf")) { return Inf; }
    if (CMP("Sup")) { return Sup; }
    if (CMP("LimInf")) { return LimInf; }
    if (CMP("LimSup")) { return LimSup; }
    if (CMP("LimInfAvg")) { return LimInfAvg; }
    if (CMP("LimSupAvg")) { return LimSupAvg; }
    if (CMP("Avg")) { return Avg; }
#undef CMP

    std::cerr << "Unknown value function: " << str << "\n";
    abort();
}

const char *valueFunctionToStr(value_function_t v) {
  switch(v) {
    case Inf: return "Inf";
    case Sup: return "Sup";
    case LimInf: return "LimInf";
    case LimSup: return "LimSup";
    case LimInfAvg: return "LimInfAvg";
    case LimSupAvg: return "LimSupAvg";
    case Avg: return "Avg";
  }
  abort();
}

