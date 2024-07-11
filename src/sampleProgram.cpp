#include <iostream>
#include <vector>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Monitor.h"

int main(int argc, char **argv) {

	Automaton* A = new Automaton("samples/A.txt");
	A->print();
	Automaton* B = new Automaton("samples/test3.txt");
	B->print();

	std::cout << "Taking Val = LimInf" << std::endl;

	std::cout << "Is A(w) >= 4 for some word w? " << A->isNonEmpty(LimInf, 4) << std::endl;
	std::cout << "Is A(w) >= 4 for every word w? " << A->isUniversal(LimInf, 4) << std::endl;
	std::cout << "Does A define a constant function? " << A->isConstant(LimInf) << std::endl;
	std::cout << "Is A included in B? (calling quantitative antichain) : " << A->isIncludedIn(B, LimInf) << std::endl;
	std::cout << "Is A included in B? (calling boolean antichain) : " << A->isIncludedIn(B, LimInf, true) << std::endl;
	std::cout << "Is A safe? " << A->isSafe(LimInf) << std::endl;
	std::cout << "Is A live? " << A->isLive(LimInf) << std::endl;
	
	std::cout << "Top value of A = " << A->getTopValue(LimInf) << std::endl;
	std::cout << "Bottom value of A = " << A->getBottomValue(LimInf) << std::endl;
	std::cout << "Top value of B = " << B->getTopValue(LimInf) << std::endl;
	std::cout << "Bottom value of B = " << B->getBottomValue(LimInf) << std::endl;

	Automaton* safe_A = Automaton::safetyClosure(A, LimInf);
	safe_A->print();
	std::cout << "Does Safe(A) define a constant function? " << safe_A->isConstant(LimInf) << std::endl;

	std::cout << "Is B safe? " << B->isSafe(LimInf) << std::endl;
	std::cout << "Is B live? " << B->isLive(LimInf) << std::endl;

	Automaton* safe_B = Automaton::safetyClosure(B, LimInf);
	safe_B->print();
	Automaton* live_B = Automaton::livenessComponent_deterministic(B, LimInf);
	live_B->print();

	std::cout << "Is Safe(B) safe? " << safe_B->isSafe(LimInf) << std::endl;
	std::cout << "Is Live(B) live? " << live_B->isLive(LimInf) << std::endl;

	delete safe_B;
	delete live_B;
	delete safe_A;
	delete B;
	delete A;

	return EXIT_SUCCESS;
}


