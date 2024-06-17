#include <iostream>
#include "Automaton.h"
#include "Word.h"
#include "FORQ/TargetOf.h"
#include "FORQ/FixpointLoop.h"
#include "FORQ/FixpointStem.h"
#include "FORQ/ContextOf.h"
#include "FORQ/StateRelation.h"
#include "FORQ/PostContextVariable.h"
#include "FORQ/PostTargetVariable.h"


int main(int argc, char **argv) {
	printf("--------------------------------------------\n");

	Automaton* toto = new Automaton("./samples/bigb_SUBSET.txt");
	std::cout << std::endl << toto->toString() << std::endl;

	printf("--------------------------------------------\n");
/*
	Word* period = new Word(toto->getAlphabet()->at(0));
	TargetOf* set = new TargetOf();
	State* state = toto->getStates()->at(1);
	set->add(state);
	weight_t value = toto->membership(set, period);
	std::cout << std::endl << "[" << state->getName() << "]({" << period->toString() << "}) = " << value << std::endl;

	printf("--------------------------------------------\n");
*/

	printf("MAX prefixes fix-point: computing ..\r");
	int iter_W = 1;
	FixpointStem* postIrev = new FixpointStem(toto->getInitial(), toto->getInitial(), true);
	while (postIrev->apply()) {
		iter_W++;
	}
	printf("MAX prefixes fix-point: done ( %d iterations )\n", iter_W);
	unsigned int x = 0;
	for (unsigned int state_id = 0; state_id < toto->getStates()->size(); ++state_id) {
		x += postIrev->getSetOfTargets(toto->getStates()->at(state_id))->size();
	}
	printf("Size %u\n", x);

	printf("MIN prefixes fix-point: computing ..\r");
	int iter_U = 1;
	FixpointStem* postI = new FixpointStem(toto->getInitial(), toto->getInitial(), false);
	while (postI->apply()) {
		iter_U++;
	}
	printf("MIN prefixes fix-point: done ( %d iterations )\n", iter_U);
	unsigned int y = 0;
	for (unsigned int state_id = 0; state_id < toto->getStates()->size(); ++state_id) {
		y += postI->getSetOfTargets(toto->getStates()->at(state_id))->size();
	}
	printf("Size %u\n", y);


/*

	printf("--------------------------------------------\n");
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
*/

	// std::cout << "Included(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;
	// std::cout << "Included(B,A): " << B->isIncludedIn_det(Sup, A) << std::endl;
	// std::cout << "Equivalent(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;

	return EXIT_SUCCESS;
}
