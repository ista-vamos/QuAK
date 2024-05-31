
#include "Weight.h"
#include "utility.h"

int ID_of_Weights = 0;
template <typename T_value>
void Weight<T_value>::Weight::RESET () { ID_of_Weights = 0; }


template <typename T_value>
Weight<T_value>::~Weight() {
	delete_verbose("@Memory: Weight deleted (%s)\n", this->toString().c_str());
}


template <typename T_value>
Weight<T_value>::Weight(T_value value) : my_id(ID_of_Weights++), value(value) {}

template <typename T_value>
Weight<T_value>::Weight(Weight<weight_t>* weight) : my_id(weight->my_id), value(weight->value) {}


template <typename T_value>
T_value Weight<T_value>::getValue() const { return this->value; }


template <typename T_value>
const int Weight<T_value>::getId() const { return this->my_id; }


template <typename T_value>
std::string Weight<T_value>::Weight::toString (Weight* weight) { return weight->toString(); }


template <typename T_value>
std::string Weight<T_value>::toString() const { return std::to_string(this->value); }

