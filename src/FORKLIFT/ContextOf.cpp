

#include "ContextOf.h"
#include "inclusion.h"



ContextOf::~ContextOf() {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		delete this->at(weight_id);
	}
}



ContextOf::ContextOf(unsigned int capacity) : MapArray<StateRelation*>(capacity) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		this->insert(weight_id, new StateRelation());
	}
}



ContextOf::ContextOf (ContextOf* currentB, Symbol* symbol) : MapArray<StateRelation*>(currentB->size()) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		this->insert(weight_id, new StateRelation());
	}

	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		for (std::pair<State*,TargetOf*> pairB : *(currentB->at(weight_id))) {
			for (State* fromB: *(pairB.second)) {
				for (Edge* edgeB : *(fromB->getSuccessors(symbol->getId()))) {
				  // #ifdef INCLUSION_SCC_SEARCH_ACTIVE
				  // if (edgeB->getFrom()->getTag() != edgeB->getTo()->getTag()) continue;
				  // #endif
					weight_t max_weight_id = std::max(weight_id, edgeB->getWeight()->getId());
					add(pairB.first, edgeB->getTo(), max_weight_id);
					// -- since weight are sorted
					// -- id_1 < id_2 <==>  value_1 < value_2
				}
			}
		}
	}
}


/*
void ContextOf::print () {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		printf("weight_id %u\n", weight_id);
		this->at(weight_id)->print();
	}
}
*/





#ifdef CONTEXT_REDUNDANCY_ACTIVE//##############################

void ContextOf::add (State* fromB, State* toB, unsigned int weight_id) {
	for (unsigned int id = 0; id <= weight_id; ++id) {
		this->at(id)->add(fromB, toB);
	}
}



bool ContextOf::smaller_than (ContextOf* other) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		if (this->at(weight_id)->smaller_than(other->at(weight_id)) == false) {
			return false;
		}
	}
	return true;
}


#else//##############################


void ContextOf::add (State* fromB, State* toB, unsigned int weight_id) {
	this->at(weight_id)->add(fromB, toB);
}



bool ContextOf::smaller_than (ContextOf* other) {
	for (unsigned int x_id = 0; x_id < this->size(); ++x_id) {
		for (std::pair<State*, TargetOf*> pair : *(this->at(x_id))) {
			for (State* state : *(pair.second)) {
				bool flag = false;
				for (unsigned int y_id = x_id; y_id < this->size(); ++y_id) {
					if (other->at(y_id)->contains(pair.first) == false) continue;
					if (other->at(y_id)->at(pair.first)->contains(state) == false) continue;
					flag = true;
					break;
				}
				if (flag == false) return false;
			}
		}
	}
	return true;
}

#endif//##############################




