GCC = gcc
CFLAG = -g -Wall

GPP = g++
CPPFLAG = -g -Wall

SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.cpp)

OBJDIR = obj
OBJ = $(SRC:$(SRCDIR)/%=$(OBJDIR)/%.o)

BINDIR = bin
BIN = chasm.out


default: all


exe:
	./$(BINDIR)/$(BIN)


all: $(BINDIR)/$(BIN)


$(BINDIR)/$(BIN): $(OBJ)
	$(GPP) $(CPPFLAG) $^ -o $@


$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	$(GCC) $(CFLAG) -c ./$< -o $@


$(OBJDIR)/%.cpp.o: $(SRCDIR)/%.cpp
	$(GPP) $(CPPFLAG) -c $< -o $@


.PHONY: clean


clean:
	rm -f $(wildcard $(OBJDIR)/*)


