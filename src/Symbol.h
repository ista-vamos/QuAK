
#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <string>


class Symbol {
private:
	const unsigned int my_id;
	std::string name;
public:
	Symbol(std::string name);
	Symbol(Symbol* symbol);
	Symbol(const Symbol& other);
	~Symbol();
	std::string getName() const;
	unsigned int getId() const;
	static void RESET();
	static void RESET(unsigned int n);
	static std::string toString(Symbol* symbol);
	std::string toString() const;
};



#endif /* SYMBOL_H_ */
