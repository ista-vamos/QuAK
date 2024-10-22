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
    std::vector<int> states = {2, 4, 8};
    std::vector<int> letters = {2, 4};

    std::string directory = "../samples/rand"; // Current directory
    
    for (int num_states : states) {
        for (int num_letters : letters) {
            for (int id = 1; id <= 1000; ++id) {
                std::ostringstream filename;
                filename << num_states << "_" << num_letters << "_" 
                         << std::setw(4) << std::setfill('0') << id << ".txt";
                
                std::cout << "Processing file: " << filename.str() << std::endl;
                
                Automaton* A = new Automaton(directory + "/" + filename.str());
                // A->print();

                // weight_t topxx = A->getTopValue(LimInfAvg);

                // Automaton* B = A->livenessComponent_prefixIndependent(A, LimInfAvg);
                // // B->print();
                // bool flag = B->isLive(LimInf);
                // delete B;
                // delete A;
                // if (!flag) {
                //     std::cout << "Error: " << filename.str() << std::endl;
                // }

                UltimatelyPeriodicWord* witness = new UltimatelyPeriodicWord();
                weight_t top = A->getTopValue(LimInfAvg, &witness);

                std::cout << "Witness to top value:" << std::endl;
                std::cout << "Prefix: " << witness->prefix->toString() << std::endl;
                std::cout << "Cycle: " << witness->cycle->toString() << std::endl;
                
                weight_t val = A->computeValue(LimInfAvg, witness);

                delete witness->prefix;
                delete witness->cycle;
                delete witness;
                delete A;

                if (val != top) {
                  std::cout << "Error: " << filename.str() << std::endl;
                  return -1;
                }
            }
        }
    }

    return 0;
}

// int main(int argc, char **argv) {

// 	// Automaton* A = new Automaton("../samples/qwer.txt");
// 	// A->print();
// 	// Automaton* B = new Automaton("../samples/qwer2.txt");
// 	// // Automaton* B = A->livenessComponent_prefixIndependent(A, LimSup);
// 	// B->print();
  
// 	// // std::cout << "Is A live? " << A->isLive(LimInfAvg) << std::endl;
// 	// // std::cout << "Is B live? " << B->isLive(LimInfAvg) << std::endl;
  
//   // UltimatelyPeriodicWord* witness = new UltimatelyPeriodicWord();
//   // bool flag = A->isIncludedIn(B, LimSup, false, &witness);

//   // if (!flag) {
//   //   std::cout << "Witness found (counterexample to inclusion):" << std::endl;
//   //   std::cout << "Prefix: " << witness->prefix->toString() << std::endl;
//   //   std::cout << "Cycle: " << witness->cycle->toString() << std::endl;

//   //   delete witness->prefix;
//   //   delete witness->cycle;
//   // }
// 	// delete witness;
//   // delete B;
// 	// delete A;

//   Automaton* A = new Automaton("../samples/qwer.txt");
// 	A->print();
  
//   UltimatelyPeriodicWord* witness = new UltimatelyPeriodicWord();
//   weight_t top = A->getTopValue(LimSup, &witness);

//   if (true) {
//     std::cout << "Witness to top value:" << std::endl;
//     std::cout << "Prefix: " << witness->prefix->toString() << std::endl;
//     std::cout << "Cycle: " << witness->cycle->toString() << std::endl;

//     delete witness->prefix;
//     delete witness->cycle;
//   }
//   delete witness;
// 	delete A;

// 	return EXIT_SUCCESS;
// }
