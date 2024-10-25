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


// int main() {
//     std::vector<int> states = {3};
//     std::vector<int> letters = {2};

//     std::string directory = "../samples/rand";
    
//     for (int num_states : states) {
//         for (int num_letters : letters) {
//             for (int id = 1; id <= 10000; ++id) {
//                 std::ostringstream filename;
//                 filename << num_states << "_" << num_letters << "_" 
//                          << std::setw(4) << std::setfill('0') << id << ".txt";
                
//                 std::cout << "Processing file: " << filename.str() << std::endl;
                
//                 Automaton* A = new Automaton(directory + "/" + filename.str());
//                 A->print();

//                 // weight_t topxx = A->getTopValue(LimInfAvg);

//                 // Automaton* B = A->livenessComponent_prefixIndependent(A, LimInfAvg);
//                 // // B->print();
//                 // bool flag = B->isLive(LimInf);
//                 // delete B;
//                 // delete A;
//                 // if (!flag) {
//                 //     std::cout << "Error: " << filename.str() << std::endl;
//                 // }

//                 UltimatelyPeriodicWord* witness = new UltimatelyPeriodicWord();
//                 // weight_t top = A->getTopValue(LimInfAvg, &witness);
//                 bool flag = A->isConstant(LimInfAvg, &witness);

//                 weight_t top = A->getTopValue(LimInfAvg);
//                 std::cout << "Top:" << top << std::endl;
//                 weight_t valWitness = top - 1;
//                 if (!flag) {
//                     valWitness = A->computeValue(LimInfAvg, witness);
//                     std::cout << "Witness: " << witness->prefix->toString() << " . " << witness->cycle->toString() << std::endl;
//                     std::cout << "Value: " << valWitness << std::endl;
//                 }
                
//                 delete witness->prefix;
//                 delete witness->cycle;
//                 delete witness;
//                 delete A;

//                 if (!flag && top <= valWitness) {
//                     std::cout << "ERROR" << std::endl;
//                 }

//             }
//         }
//     }

//     return 0;
// }

std::string val2str(value_function_t f) {
    if (f == Inf) {
        return "Inf";
    }
    if (f == Sup) {
        return "Sup";
    }
    if (f == LimInf) {
        return "LimInf";
    }
    if (f == LimSup) {
        return "LimSup";
    }
    if (f == LimInfAvg) {
        return "LimInfAvg";
    }
    if (f == LimSupAvg) {
        return "LimInfAvg";
    }
    
    return "";
}

int main() {
    std::vector<int> states = {2,3,4,5,6,7,8};
    std::vector<int> letters = {2,3,4};
    // std::vector<int> states = {2};
    // std::vector<int> letters = {2};
    // std::vector<value_function_t> valueFunctions = {Inf};
    std::vector<value_function_t> valueFunctions = {Inf, Sup, LimInf, LimSup};
    // std::vector<value_function_t> valueFunctions = {Inf, Sup, LimInf, LimSup, LimInfAvg, LimSupAvg};

    std::string directory = "../samples/randBig";
    
    for (value_function_t f : valueFunctions) {
        for (int num_states : states) {
            for (int num_letters : letters) {
                for (int id = 1; id <= 10000; ++id) {
                    std::ostringstream filename;
                    filename << num_states << "_" << num_letters << "_" 
                            << std::setw(4) << std::setfill('0') << id << ".txt";
                    
                    std::cout << "Processing automaton: " <<  val2str(f) << " " << filename.str() << std::endl;
                    
                    Automaton* A = new Automaton(directory + "/" + filename.str());
                    // A->print();

                    // testing the top value witness
                    /*
                    UltimatelyPeriodicWord* witness = new UltimatelyPeriodicWord();
                    weight_t top = A->getTopValue(f, &witness);
                    weight_t valWitness = A->computeValue(f, witness);

                    delete witness->prefix;
                    delete witness->cycle;
                    delete witness;
                    delete A;

                    if (top != valWitness) {
                        std::cout << "ERROR" << std::endl;
                        return -1;
                    }
                    */

                    // testing the universality witness (which calls inclusion)
                    UltimatelyPeriodicWord* witness = new UltimatelyPeriodicWord();
                    weight_t top = A->getTopValue(f);
                    bool flag = A->isUniversal(f, top, &witness);

                    weight_t valWitness = top - 1;
                    if (!flag) {
                        valWitness = A->computeValue(f, witness);
                    }

                    delete witness;
                    delete A;

                    if (valWitness >= top) {
                        std::cout << "ERROR" << std::endl;
                        return -1;
                    }


                    // Automaton* B = Automaton::safetyClosure(A, Sup);
                    // bool flag = B->isSafe(LimSup);

                    // delete B;
                    // delete A;

                    // if (!flag) {
                    //     std::cout << "ERROR" << std::endl;
                    // }


                    // Automaton* AA = Automaton::toLimSup(A, Inf);
                    // AA->print();
                    // UltimatelyPeriodicWord* witness = new UltimatelyPeriodicWord();
                    // weight_t top = AA->getTopValue(LimSup, &witness);
                    // std::cout << "Top:" << top << std::endl;
                    // std::cout << "Witness: " << witness->prefix->toString() << " . " << witness->cycle->toString() << std::endl;
                    // weight_t valWitness = A->computeValue(Inf, witness);
                    // std::cout << "Value: " << valWitness << std::endl;
                    // delete witness->prefix;
                    // delete witness->cycle;
                    // delete witness;
                    // delete AA;
                    // delete A;


                }
            }
        }
    }

    return 0;
}