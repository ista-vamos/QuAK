
#ifndef WEIGHT_H_
#define WEIGHT_H_

#include <string>
#include <cmath>
#include <iostream>

class weight_t {
public:
  // int, float, double
  using T = float;

private:
  static constexpr T EPSILON = WEIGHT_EQ_EPSILON;

  T value{0};

  friend std::hash<weight_t>;
  friend std::ostream& operator<<(std::ostream &os, weight_t);
  friend std::istream& operator>>(std::istream &os, weight_t&);

public:
  constexpr weight_t() = default;
  //weight_t(weight_t&) = default;
  constexpr weight_t(T val) : value(val){};
  //weight_t(weight_t&&) = default;

  weight_t operator+(const weight_t rhs) const { return weight_t(value + rhs.value); }
  weight_t operator-(const weight_t rhs) const { return weight_t(value - rhs.value); }
  weight_t operator*(const weight_t rhs) const { return weight_t(value * rhs.value); }
  weight_t operator/(const weight_t rhs) const { return weight_t(value / rhs.value); }

  weight_t operator+=(const weight_t rhs) { value += rhs.value; return *this;}
  weight_t operator-=(const weight_t rhs) { value -= rhs.value; return *this;}

  weight_t operator--() { --value; return *this; }
  weight_t operator--(int) { auto old = value; --value; return weight_t(old); }
  weight_t operator++() { ++value; return *this; }
  weight_t operator++(int) { auto old = value; ++value; return weight_t(old); }

  bool operator<(weight_t rhs) const { return value < rhs.value; }
  bool operator>(weight_t rhs) const { return value > rhs.value; }
  bool operator<=(weight_t rhs) const { return value <= rhs.value; }
  bool operator>=(weight_t rhs) const { return value >= rhs.value; }

  weight_t operator-() const { return weight_t(-value); }

  explicit operator int() const { return static_cast<int>(value); }
  explicit operator float() const { return value; }

  float to_float() const { return value; }

  bool operator==(weight_t rhs) const { return fabs(value - rhs.value) < EPSILON; }
  bool operator!=(weight_t rhs) const { return fabs(value - rhs.value) >= EPSILON; }

  std::string to_string() const { return std::to_string(value); }

};

std::ostream& operator<<(std::ostream &os, weight_t);
std::istream& operator>>(std::istream &os, weight_t&);

namespace std {
  std::string to_string(weight_t x);

  template <> struct hash<weight_t> {
    size_t operator()(const weight_t x) const {
      return x.value;
    }
  };
}

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
	unsigned int getId() const;
	static std::string toString(Weight* weight);
	std::string toString() const;
};


#endif /* WEIGHT_H_ */
