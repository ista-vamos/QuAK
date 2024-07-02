
#ifndef WEIGHT_H_
#define WEIGHT_H_

#include <string>

// int, float, double
// TODO
typedef float weight_t;

class Weight {
private:
	const unsigned int my_id;
	weight_t value;
public:
	~Weight();
	Weight(weight_t value);
	Weight(Weight* weight);
	static void RESET();
	weight_t getValue() const;
	void setValue(weight_t value);
	const unsigned int getId() const;
	static std::string toString(Weight* weight);
	std::string toString() const;
};


#endif /* WEIGHT_H_ */
