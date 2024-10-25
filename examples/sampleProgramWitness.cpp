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

    UltimatelyPeriodicWord* witEmp;
    bool flagNonEmp = A->isNonEmpty(LimInfAvg, 5, &witEmp);

    if (flagNonEmp) {
        weight_t valWitness = A->computeValue(LimInfAvg, witEmp);
        std::cout << "Witness to non-emptiness of A w.r.t. value 5: " << witEmp->toString() << std::endl;
        std::cout << "Value: " << valWitness << std::endl;
    }

    delete witEmp;

    UltimatelyPeriodicWord* witSafe;
    bool flagSafe = A->isSafe(LimInfAvg, &witSafe);

    if (!flagSafe) {
        weight_t valWitness = A->computeValue(LimInfAvg, witSafe);
        weight_t valWitnessClosure = B->computeValue(LimInfAvg, witSafe);
        std::cout << "Witness to non-safety of A: " << witSafe->toString() << std::endl;
        std::cout << "Value on A: " << valWitness << std::endl;
        std::cout << "Value on SafetyClosure(A): " << valWitnessClosure << std::endl;
    }

    delete witSafe;
    
    delete C;
    delete B;
    delete A;

    return 0;
}
