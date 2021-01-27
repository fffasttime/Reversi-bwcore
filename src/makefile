CXXFLAGS = -std=c++14 -g -Wall -DDEBUG
objects = util.o board.o bitptn.o evalptn.o

all: $(objects) main.o
	g++ $(objects) main.o -o bwcore1.5.exe

test: $(objects) test.o
	g++ $(objects) test.o -o test.exe

$(objects): util.h
test.o: board.h bitptn.h evalptn.h
main.o: board.h bitptn.h
evalptn.o: board.h bitptn.h
board.o: bitptn.h
bitptn.o:
util.o:

.PHONY: clean
clean: 
	rm -f bwcore1.5.exe test.exe *.o
