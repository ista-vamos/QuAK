
#include "inclusion.h"

#include "../Map.h"
#include "../Word.h"
#include "TargetOf.h"
#include "FixpointLoop.h"
#include "FixpointStem.h"
#include "ContextOf.h"
#include "StateRelation.h"
#include "PostContextVariable.h"
#include "PostTargetVariable.h"



SetStd<std::pair<State*,std::pair<unsigned int, bool>>> S;
SetStd<State*> P;



bool relevance_test (TargetOf* W, ContextOf* V, const Automaton* B) {
	for (auto iter = V->at(0)->begin(); iter != V->at(0)->end(); ++iter) {
		if (iter->second->smaller_than(W) == false) return false;
	}
	return true;
}



bool fast_iterable_final_product (State* from, unsigned int i, Word* period) {
	S.insert(std::pair<State*,std::pair<unsigned int, bool>>(from, std::pair<unsigned int, bool>(i, true)));

	for (Edge* edge : *(from->getSuccessors(period->at(i)->getId()))) {
		unsigned int ii = (i+1 == period->getLength())?0:i+1;
		if (P.contains(edge->getTo())) return true;
		if (S.contains(std::pair<State*,std::pair<unsigned int, bool>>(edge->getTo(), std::pair<unsigned int, bool>(ii, true))) == false) {
			if (fast_iterable_final_product(edge->getTo(), ii, period) == true) return true;
		}
	}

	return false;
}



bool fast_reachable_final_product (State* from, unsigned int i, Word* period, weight_t threshold) {
	S.insert(std::pair<State*,std::pair<unsigned int, bool>>(from, std::pair<unsigned int, bool>(i, false)));
	P.insert(from);

	for (Edge* edge : *(from->getSuccessors(period->at(i)->getId()))) {
		unsigned int ii = (i+1 == period->getLength())?0:i+1;
		if (S.contains(std::pair<State*,std::pair<unsigned int, bool>>(edge->getTo(), std::pair<unsigned int, bool>(ii, false))) == false) {
			if (fast_reachable_final_product(edge->getTo(), ii, period, threshold) == true) return true;
		}
	}


	for (Edge* edge : *(from->getSuccessors(period->at(i)->getId()))) {
		if (edge->getWeight()->getValue() >= threshold) {
			unsigned int ii = (i+1 == period->getLength())?0:i+1;
			if (P.contains(edge->getTo())) return true;
			if (S.contains(std::pair<State*,std::pair<unsigned int, bool>>(edge->getTo(), std::pair<unsigned int, bool>(ii, true))) == false) {
				if (fast_iterable_final_product(edge->getTo(), ii, period) == true) return true;
			}
		}
	}

	P.erase(from);

	return false;
}


bool fast_membership (TargetOf* U, Word* period, weight_t threshold) {
	for (State* start : *U) {
		S.clear(); P.clear();
		if (fast_reachable_final_product(start, 0, period, threshold) == true) return true;
	}
	return false;
}



bool inclusion (const Automaton* A, const Automaton* B)  {

	unsigned int iter_W = 1;
	printf("MAX prefixes fix-point: computing ... ( %u iterations )\r", iter_W);fflush(stdout);
	FixpointStem* postIrev = new FixpointStem(A->getInitial(), B->getInitial(), true);
	while (postIrev->apply()) {
		iter_W++;
		printf("MAX prefixes fix-point: computing ... ( %u iterations )\r", iter_W);fflush(stdout);
	}
	printf("MAX prefixes fix-point: done ( %u iterations )          \n", iter_W);fflush(stdout);
	/*
	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		State* stateA = A->getStates()->at(stateA_id);
		SetStd<std::pair<TargetOf*, Word*>>* targets = postIrev->getSetOfTargetsOrNULL(stateA);
		printf("%s ->\n", stateA->getName().c_str());
		for (std::pair<TargetOf*, Word*> pair : *targets) {
			printf("\t %s\n", pair.second->toString().c_str());
		}
	}
	*/


	unsigned int iter_U = 1;
	printf("MIN prefixes fix-point: computing ... ( %u iterations )\r", iter_U);fflush(stdout);
	FixpointStem* postI = new FixpointStem(A->getInitial(), B->getInitial(), false);
	while (postI->apply()) {
		iter_U++;
		printf("MIN prefixes fix-point: computing ... ( %u iterations )\r", iter_U);fflush(stdout);
	}
	printf("MIN prefixes fix-point: done ( %u iterations )          \n", iter_U);fflush(stdout);
	/*
	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		State* stateA = A->getStates()->at(stateA_id);
		SetStd<std::pair<TargetOf*, Word*>>* targets = postI->getSetOfTargetsOrNULL(stateA);
		printf("%s ->\n", stateA->getName().c_str());
		for (std::pair<TargetOf*, Word*> pair : *targets) {
			printf("\t %s\n", pair.second->toString().c_str());
		}
	}
	*/


	unsigned int final_counter = 0;
	unsigned int membership_counter = 0;
	unsigned int call_V = 0;
	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		if (A->getStates()->at(stateA_id)->getMaxWeightValue() < 1) continue; //fixme:
		//if (A->getStates()->at(stateA_id)->getMaxWeightValue() < B->getMinWeightValue()) continue;

		final_counter++;
		State* stateA = A->getStates()->at(stateA_id);
		printf("State: %s (%u, %u/%u)\n",
				stateA->getName().c_str(),
				final_counter,
				stateA_id,
				A->getStates()->size()
		);fflush(stdout);

		unsigned int max_counter = 0;
		SetStd<std::pair<TargetOf*, Word*>>* setW = postIrev->getSetOfTargetsOrNULL(stateA);
		if (setW == NULL) continue;
		for (std::pair<TargetOf*, Word*> pairW : *setW) {
			call_V++;
			max_counter++;

			TargetOf* W = pairW.first;
			Word* word_of_W = pairW.second; // unused
			printf("\tMax Prefix: %s (%u/%u)\n",
					word_of_W->toString().c_str(),
					max_counter,
					setW->size()
			);fflush(stdout);


			unsigned int iter_V_local = 1;
			printf("\t\tPeriod fix-point: computing ... ( %u iterations )\r", iter_V_local);fflush(stdout);
			FixpointLoop* postF = new FixpointLoop(stateA, W, B->getWeights()->size());
			while (postF->apply()) {
				iter_V_local++;
				printf("\t\tPeriod fix-point: computing ... ( %u iterations )\r", iter_V_local);fflush(stdout);
			}
			printf("\t\tPeriod fix-point: done ( %u iterations )          \n", iter_V_local);fflush(stdout);

			unsigned int period_counter = 0;
			SetStd<std::pair<ContextOf*, std::pair<Word*,weight_t>>>* setV = postF->getSetOfContextsOrNULL(stateA);

			if (setV == NULL) {
				delete postF;
				continue;
			}
			for (std::pair<ContextOf*, std::pair<Word*,weight_t>> pairV : *setV) {
				period_counter++;
				ContextOf* V = pairV.first;
				Word* word_of_V = pairV.second.first;
				weight_t valueA = pairV.second.second;

				printf("\t\tPeriod: %s (%u/%u)\n",
						pairV.second.first->toString().c_str(),
						period_counter,
						setV->size()
				);fflush(stdout);


				if (relevance_test(W, V, B) == false) continue;
				SetStd<std::pair<TargetOf*, Word*>>* setU = postI->getSetOfTargetsOrNULL(stateA);
				if (setU == NULL) continue;
				for (std::pair<TargetOf*, Word*> pairU : *setU) {
					TargetOf* U = pairU.first;
					Word* word_of_U = pairU.second;

					if (U->smaller_than(W) == true) {
						membership_counter++;
						printf("\t\t\tMEMBERSHIP %u: %s < %s\n",
								membership_counter,
								word_of_U->toString().c_str(),
								pairW.second->toString().c_str()
						);fflush(stdout);

						if (fast_membership(U, word_of_V, valueA) == false) {
							printf("witness: %s cycle{ %s }\n", word_of_U->toString().c_str(), word_of_V->toString().c_str());
							delete postF;
							delete postI;
							delete postIrev;
							return false;
						}
					}
				}
			}
			delete postF;
		}
	}
	delete postI;
	delete postIrev;

	printf("TRUE\n");
	return true;
}



void debug_test() {
	Automaton* toto;
	Automaton* titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUBSET.txt");
	titi = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUPERSET.txt", toto);
	std::cout << "All_positive_numbers_have_a_predecessor" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUPERSET.txt");
	 toto = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUBSET.txt", titi);
	 std::cout << "All_positive_numbers_have_a_predecessor" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUBSET.txt");
	 titi = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUPERSET.txt", toto);
	 std::cout << "All_Sturmian_words_contain_cubes" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUPERSET.txt");
	 toto = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUBSET.txt", titi);
	 std::cout << "All_Sturmian_words_contain_cubes" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUBSET.txt");
	 titi = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUPERSET.txt", toto);
	 std::cout << "All_Sturmian_words_start_with_arbitarily_long_palindromes" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUPERSET.txt");
	 toto = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUBSET.txt", titi);
	 std::cout << "All_Sturmian_words_start_with_arbitarily_long_palindromes" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/bakeryV3_SUBSET.txt");
	 titi = new Automaton("./samples/bakeryV3_SUPERSET.txt", toto);
	 std::cout << "bakeryV3" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/bakeryV3_SUPERSET.txt");
	 toto = new Automaton("./samples/bakeryV3_SUBSET.txt", titi);
	 std::cout << "bakeryV3" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/bigb_SUBSET.txt");
	 titi = new Automaton("./samples/bigb_SUPERSET.txt", toto);
	 std::cout << "bigb" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/bigb_SUPERSET.txt");
	 toto = new Automaton("./samples/bigb_SUBSET.txt", titi);
	 std::cout << "bigb" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/BuchiCegarLoopAbstraction_SUBSET.txt");
	 titi = new Automaton("./samples/BuchiCegarLoopAbstraction_SUPERSET.txt", toto);
	 std::cout << "BuchiCegarLoopAbstraction" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/BuchiCegarLoopAbstraction_SUPERSET.txt");
	 toto = new Automaton("./samples/BuchiCegarLoopAbstraction_SUBSET.txt", titi);
	 std::cout << "BuchiCegarLoopAbstraction" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/example_SUBSET.txt");
	 titi = new Automaton("./samples/example_SUPERSET.txt", toto);
	 std::cout << "example" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/example_SUPERSET.txt");
	 toto = new Automaton("./samples/example_SUBSET.txt", titi);
	 std::cout << "example" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/fischerV3_SUBSET.txt");
	 titi = new Automaton("./samples/fischerV3_SUPERSET.txt", toto);
	 std::cout << "fischerV3" << std::endl;
	 std::cout << "NOT COMPUTED" << std::endl;//inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/fischerV3_SUPERSET.txt");
	 toto = new Automaton("./samples/fischerV3_SUBSET.txt", titi);
	 std::cout << "fischerV3" << std::endl;
	 std::cout << "NOT COMPUTED" << std::endl;//inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/Odd_and_even_work_as_expected_SUBSET.txt");
	 titi = new Automaton("./samples/Odd_and_even_work_as_expected_SUPERSET.txt", toto);
	 std::cout << "Odd_and_even_work_as_expected" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/Odd_and_even_work_as_expected_SUPERSET.txt");
	 toto = new Automaton("./samples/Odd_and_even_work_as_expected_SUBSET.txt", titi);
	 std::cout << "Odd_and_even_work_as_expected" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/peterson_SUBSET.txt");
	 titi = new Automaton("./samples/peterson_SUPERSET.txt", toto);
	 std::cout << "peterson" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/peterson_SUPERSET.txt");
	 toto = new Automaton("./samples/peterson_SUBSET.txt", titi);
	 std::cout << "peterson" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/slides_example_SUBSET.txt");
	 titi = new Automaton("./samples/slides_example_SUPERSET.txt", toto);
	 std::cout << "slides_example" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/slides_example_SUPERSET.txt");
	 toto = new Automaton("./samples/slides_example_SUBSET.txt", titi);
	 std::cout << "slides_example" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/Specal_factors_are_unique_SUBSET.txt");
	 titi = new Automaton("./samples/Specal_factors_are_unique_SUPERSET.txt", toto);
	 std::cout << "Specal_factors_are_unique" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/Specal_factors_are_unique_SUPERSET.txt");
	 toto = new Automaton("./samples/Specal_factors_are_unique_SUBSET.txt", titi);
	 std::cout << "Specal_factors_are_unique" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");

	 toto = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUBSET.txt");
	 titi = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUPERSET.txt", toto);
	 std::cout << "The_lazy_Ostrowski_representation_is_unique" << std::endl;
	 inclusion(toto, titi);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");

	 titi = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUPERSET.txt");
	 toto = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUBSET.txt", titi);
	 std::cout << "The_lazy_Ostrowski_representation_is_unique" << std::endl;
	 inclusion(titi, toto);
	 delete toto;
	 delete titi;

	 printf("--------------------------------------------\n");
	 printf("--------------------------------------------\n");
}


