GCC = gcc
# CFLAG = -g -O3 -Wall
CFLAG = -g -Wall

GPP = g++
# CPPFLAG = -g -O3 -Wall
INCLUSION_OPTS=-DINCLUSION_SCC_SEARCH_ACTIVE -DCONTEXT_REDUNDANCY_ACTIVE
CPPFLAG = -g -Wall $(INCLUSION_OPTS)

SRCDIR = src
SRC0 = $(filter-out $(SRCDIR)/quak-main.cpp, $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.cpp))
SRC1 = $(wildcard $(SRCDIR)/*/*.c) $(wildcard $(SRCDIR)/*/*.cpp)
SRC = $(SRC0) $(SRC1)
#SRC = $(SRC0)

OBJDIR = obj
OBJ = $(SRC:$(SRCDIR)/%=$(OBJDIR)/%.o)

BINDIR = bin
BIN = chasm.out


default: all


run:
	./$(BINDIR)/$(BIN)


all: $(BINDIR)/$(BIN)


$(BINDIR)/$(BIN): $(OBJ) $(SRCDIR)/quak-main.cpp
	mkdir -p $(BINDIR)
	$(GPP) $(CPPFLAG) $^ -o $@

$(BINDIR)/inclusion: $(OBJ) experiments/inclusion.cpp
	mkdir -p $(BINDIR)
	$(GPP) -I$(SRCDIR) $(CPPFLAG) $^ -o $@


$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	$(GCC) $(CFLAG) -c $< -o $@


$(OBJDIR)/%.cpp.o: $(SRCDIR)/%.cpp
	$(GPP) $(CPPFLAG) -c $< -o $@


.PHONY: clean


clean:
	rm -f $(wildcard $(OBJDIR)/*.o) $(wildcard $(OBJDIR)/*/*.o)
	rm -f $(BINDIR)/$(BIN)


