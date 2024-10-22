#include <iostream>

#include "Weight.h"
#include "utility.h"


std::ostream& operator<<(std::ostream &os, weight_t x) {
  os << x.value;
  return os;
}

std::istream& operator>>(std::istream &os, weight_t& x) {
  os >> x.value;
  return os;
}

namespace std {
  std::string to_string(weight_t x) {
    return x.to_string();
  }
}


unsigned int ID_of_Weights = 0;
void Weight::Weight::RESET () { ID_of_Weights = 0; }


Weight::~Weight() {
	delete_verbose("@Memory: Weight deleted (%s)\n", this->toString().c_str());
}


Weight::Weight (weight_t value) : my_id(ID_of_Weights++), value(value) {}


Weight::Weight (Weight* weight) : my_id(weight->my_id), value(weight->value) {}


weight_t Weight::getValue () const { return this->value; }


void Weight::setValue (weight_t value) { this->value = value; }


unsigned int Weight::getId() const { return this->my_id; }


std::string Weight::Weight::toString (Weight* weight) { return weight->toString(); }


std::string Weight::toString() const { return std::to_string(this->value); }

uint32_t weight_t::to_bv() const {
    static_assert(sizeof(weight_t) == sizeof(uint32_t), "This code requires 32-bit weights");
    return *static_cast<const uint32_t *>(static_cast<const void *>(&value));
}

weight_t weight_t::from_bv(uint32_t val) {
    return weight_t(*static_cast<const float *>(static_cast<const void *>(&val)));
}

