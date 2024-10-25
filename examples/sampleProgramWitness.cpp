#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Monitor.h"

int main() {
    Automaton* A = new Automaton("../samples/ex.txt");
    A->print();

    Automaton* B = Automaton::safetyClosure(A, LimInfAvg);
    B->print();

    Automaton* C = Automaton::livenessComponent_prefixIndependent(A, LimInfAvg);
    C->print();

    UltimatelyPeriodicWord* witEmp = new UltimatelyPeriodicWord();
    bool flagNonEmp = A->isNonEmpty(LimInfAvg, 5, &witEmp);

    if (flagNonEmp) {
        weight_t valWitness = A->computeValue(LimInfAvg, witEmp);
        std::cout << "Witness to non-emptiness of A w.r.t. value 5: " << witEmp->prefix->toString() << " . " << witEmp->cycle->toString() << std::endl;
        std::cout << "Value: " << valWitness << std::endl;
    }

    delete witEmp->prefix;
    delete witEmp->cycle;
    delete witEmp;

    UltimatelyPeriodicWord* witSafe = new UltimatelyPeriodicWord();
    bool flagSafe = A->isSafe(LimInfAvg, &witSafe);

    if (!flagSafe) {
        weight_t valWitness = A->computeValue(LimInfAvg, witSafe);
        weight_t valWitnessClosure = B->computeValue(LimInfAvg, witSafe);
        std::cout << "Witness to non-safety of A: " << witSafe->prefix->toString() << " . " << witSafe->cycle->toString() << std::endl;
        std::cout << "Value on A: " << valWitness << std::endl;
        std::cout << "Value on SafetyClosure(A): " << valWitnessClosure << std::endl;
    }

    delete witSafe->prefix;
    delete witSafe->cycle;
    delete witSafe;
    
    delete C;
    delete B;
    delete A;

    return 0;
}