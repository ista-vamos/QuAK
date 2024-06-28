#include <iostream>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

int main(int argc, char **argv) {
/*
	Automaton* A = new Automaton("samples/test3.txt");
	A->print();

	Automaton* B = A->safetyClosure(LimSup);
	B->print();

	Automaton* C = A->livenessComponent_det(LimSup);
	C->print();
	
	delete A;
	delete B;
	delete C;
*/


	debug_test();







/*
	Automaton* A = new Automaton("./samples/test3.txt");
	std::cout << std::endl << A->toString() << std::endl;
	printf("--------------------------------------------\n");

	std::cout << "Deterministic(A): " << A->isDeterministic() << std::endl;
	std::cout << "Complete(A): " << A->isComplete() << std::endl;

	printf("--------------------------------------------\n");
	Automaton* B = A->complete(Sup); // TODO: problem with sccs
	std::cout << std::endl << B->toString() << std::endl;
	printf("--------------------------------------------\n");

	std::cout << "Deterministic(B): " << B->isDeterministic() << std::endl;
	std::cout << "Complete(B): " << B->isComplete() << std::endl;

	printf("--------------------------------------------\n");
	Automaton* C = B->complete(Sup); // TODO: problem with "complete"
	std::cout << std::endl << C->toString() << std::endl;
	printf("--------------------------------------------\n");
*/

	// std::cout << "Deterministic(B): " << B->isDeterministic() << std::endl;
	// std::cout << "Empty(A,0): " << A->isEmpty(Sup, 0) << std::endl;
	// std::cout << "Empty(A,1000): " << A->isEmpty(Sup, 1000) << std::endl;

/*
	Automaton* A = new Automaton("./samples/test2.txt");
	std::cout << std::endl << A->toString() << std::endl;
	printf("--------------------------------------------\n");
	Automaton* B = new Automaton("./samples/test3.txt");
	std::cout << std::endl << B->toString() << std::endl;
	printf("--------------------------------------------\n");

	// TODO: these are sensitive to the sink state's weight. how should we handle these? maybe it's not necessary after all
	// Fixme: I think complete should be dropped
	// Fixme: and, the product should be modified to construct only what is useful
	std::cout << "Empty(A,2): " << A->isEmpty(Inf, 2) << std::endl;
	// std::cout << "Empty(A,2): " << A->complete(Inf)->isEmpty(Inf, 2) << std::endl; // this crashes because the sink weight causes problems to top value computation
	std::cout << "Universal(A,0): " << A->isUniversal_det(Sup,0) << std::endl;
	std::cout << "Universal(A,0): " << A->complete(Sup)->isUniversal_det(Sup,0) << std::endl;


	// std::cout << "Included(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;
	// std::cout << "Included(B,A): " << B->isIncludedIn_det(Sup, A) << std::endl;
	// std::cout << "Equivalent(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;
*/
	return EXIT_SUCCESS;
}


