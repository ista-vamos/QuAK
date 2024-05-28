
#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <string>


class Symbol {
	private:
		const int my_id;
		std::string name;
	public:
		Symbol(std::string name);
		~Symbol();
		std::string getName() const;
		const int getId() const;
		static void RESET();
		static std::string toString(Symbol* symbol);
		std::string toString() const;

		/*
		bool operator< (const Symbol* other) const;
		bool operator<= (const Symbol* other) const;
		bool operator> (const Symbol* other) const;
		bool operator>= (const Symbol* other) const;
		bool operator== (const Symbol* other) const;
		bool operator!= (const Symbol* other) const;
		int cmp (const Symbol* other) const;*/
};



#endif /* SYMBOL_H_ */
