OPT = 
CXXFLAGS = -std=c++14 -g -Wall -DDEBUG $(OPT)
objects = util.o board.o bitptn.o evalptn.o search.o debugtree.o cui.o

all: $(objects) main.o
	g++ $(CXXFLAGS) $(objects) main.o -o bwcore1.5.exe

test: $(objects) test.o
	g++ $(CXXFLAGS) $(objects) test.o -o test.exe

$(objects): util.h
test.o: board.h bitptn.h evalptn.h search.h debugtree.h
main.o: board.h bitptn.h cui.h
cui.o: board.h bitptn.h search.h evalptn.h
evalptn.o: board.h bitptn.h evalptn.h
search.o: board.h bitptn.h search.h debugtree.h evalptn.h
debugtree.o: board.h bitptn.h
board.o:  board.h bitptn.h
bitptn.o: bitptn.h
util.o:

.PHONY: clean
clean: 
	rm -f bwcore1.5.exe test.exe *.o debugtree.html
