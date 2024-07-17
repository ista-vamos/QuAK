#ifndef QUAK_MONITOR_H_
#define QUAK_MONITOR_H_

#include <memory>
#include <unordered_map>

#include "Automaton.h"


class MonitorImpl {
protected:
  const Automaton *automaton;

public:
  MonitorImpl(const Automaton* A);
  virtual ~MonitorImpl() {}

  const Automaton *getAutomaton() const { return automaton; }

	virtual weight_t next(Symbol* symbol) = 0;
};


class Monitor {
  // if the monitor creates the automaton from file,
  // we store it here to free the memory later
  std::unique_ptr<Automaton> automaton_mem{nullptr};

  std::unique_ptr<MonitorImpl> monitor;
  value_function_t value_fun;
  std::unordered_map<std::string, Symbol *> symbols;


  MonitorImpl *createMonitor(const Automaton *A, value_function_t f);
  void initializeSymbols();

public:
  Monitor(const std::string &filename, value_function_t f);
  Monitor(const Automaton* A, value_function_t f);

	weight_t next(const std::string& a);
};


/* --------------------------------------------------------------- */
//  Impl
/* --------------------------------------------------------------- */

/*
class MonitorMinMax : public DeterministicMonitor {
	weight_t* top_values;
	weight_t* bot_values;

public:
  MonitorMinMax(std::string filename, value_function_t f);
  MonitorMinMax(const Automaton* A, value_function_t f);

	void next(Symbol* symbol) override;

	weight_t getLowest() const;
	weight_t getHighest() const;
};
*/


class MonitorAvg : public MonitorImpl {
  State *state;
	weight_t sum{0};
	unsigned N{0};

public:
  MonitorAvg(const Automaton* A) : MonitorImpl(A), state(A->getInitial()){
    assert(state && "No initial state");
  }

	weight_t next(Symbol* symbol) override;
};



#endif /* MONITOR_H_ */
