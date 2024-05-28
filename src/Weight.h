
#ifndef WEIGHT_H_
#define WEIGHT_H_

#include <string>

// int, float, double
typedef int weight_t;

template <typename T_value> class Weight {
private:
	const int my_id;
	T_value value;
public:
	Weight(T_value value);
	~Weight();
	static void RESET();
	//static T_value max(T_value a, T_value b);
	//static T_value min(T_value a, T_value b);
	//static std::string toString(T_value value);
	T_value getValue() const;
	const int getId() const;
	static std::string toString(Weight* weight);
	std::string toString() const;
};


template class Weight<weight_t>;


#endif /* WEIGHT_H_ */
