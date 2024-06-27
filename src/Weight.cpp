
#include "Weight.h"
#include "utility.h"

unsigned int ID_of_Weights = 0;
void Weight::Weight::RESET () { ID_of_Weights = 0; }


Weight::~Weight() {
	delete_verbose("@Memory: Weight deleted (%s)\n", this->toString().c_str());
}


Weight::Weight (weight_t value) : my_id(ID_of_Weights++), value(value) {}


Weight::Weight (Weight* weight) : my_id(weight->my_id), value(weight->value) {}


weight_t Weight::getValue () const { return this->value; }


void Weight::setValue (weight_t value) { this->value = value; }


const unsigned int Weight::getId() const { return this->my_id; }


std::string Weight::Weight::toString (Weight* weight) { return weight->toString(); }


std::string Weight::toString() const { return std::to_string(this->value); }

