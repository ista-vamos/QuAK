
#ifndef WEIGHT_H_
#define WEIGHT_H_

#include <string>

// int, float, double
typedef int weight_t;

template <typename T_value> class Weight {
private:
	const unsigned int my_id;
	T_value value;
public:
	~Weight();
	Weight(T_value value);
	Weight(Weight<weight_t>* weight);
	static void RESET();
	T_value getValue() const;
	const unsigned int getId() const;
	static std::string toString(Weight* weight);
	std::string toString() const;
};


template class Weight<weight_t>;

#endif /* WEIGHT_H_ */
