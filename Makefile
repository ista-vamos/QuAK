GCC = gcc
CFLAG = -g -Wall

GPP = g++
CPPFLAG = -g -Wall

SRCDIR = src
SRC0 = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.cpp)
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


$(BINDIR)/$(BIN): $(OBJ)
	$(GPP) $(CPPFLAG) $^ -o $@


$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	$(GCC) $(CFLAG) -c $< -o $@


$(OBJDIR)/%.cpp.o: $(SRCDIR)/%.cpp
	$(GPP) $(CPPFLAG) -c $< -o $@


.PHONY: clean


clean:
	rm -f $(wildcard $(OBJDIR)/*.o) $(wildcard $(OBJDIR)/*/*.o)
	rm -f $(BINDIR)/$(BIN)


