CPP = g++
CC  = gcc
RES =
OBJ =
LINKOBJ =
INCS =
CXXINCS =
BIN  =
CXXFLAGS = $(CXXINCS)
CFLAGES = $(INCS)

.PHONY: all all-before all-after clean clean-custom

all: all-before idlewild all-after


clean: clean-custom
	rm -f $(OBJ) $(BIN)

$(BIN): $(LINKOBJ)
	$(CPP) $(LINKOBJ) -o "idlewild" $(LIBS)


