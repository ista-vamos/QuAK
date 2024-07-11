#include <iostream>
#include <vector>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Monitor.h"

int main(int argc, char **argv) {

	// Automaton* A = new Automaton("samples/A.txt");
	// A->print();

	// std::cout << "Taking A as a LimSup automaton:" << std::endl;
	// std::cout << "Is A(w) >= 4 for some word w? " << A->isNonEmpty(LimSup, 4) << std::endl;
	// std::cout << "Is A(w) >= 4 for every word w? " << A->isUniversal(LimSup, 4) << std::endl;
	// std::cout << "Does A define a constant function? " << A->isConstant(LimSup) << std::endl;




	// Automaton* B = new Automaton("samples/test55.txt");
	// B->print();
	// Automaton* C = Automaton::toLimSup(B, LimInf);
	// C->print();

	// bool flag;
	// // flag = B->isConstant(LimInf);
	// flag = C->isConstant(LimSup);


	Automaton* B = new Automaton("samples/test55.txt");
	B->print();
	std::cout << B->getTopValue(LimSup) << std::endl;


	// std::cout << "Taking A and B as LimSup automata:" << std::endl;
	// std::cout << "Is A included in B? (calling quantitative antichain) : " << A->isIncludedIn(B, LimSup) << std::endl;
	// std::cout << "Is A included in B? (calling boolean antichain) : " << A->isIncludedIn(B, LimSup, true) << std::endl;

	// std::cout << "Taking A and B as LimInf automata:" << std::endl;
	// std::cout << "Is A included in B? (calling quantitative antichain) : " << A->isIncludedIn(B, LimInf) << std::endl;
	// std::cout << "Is A included in B? (calling boolean antichain) : " << A->isIncludedIn(B, LimInf, true) << std::endl;
	
	// Automaton* safe_A = Automaton::safetyClosure(A, LimAvg);
	// safe_A->print();

	// delete C;
	delete B;

	return EXIT_SUCCESS;
}


