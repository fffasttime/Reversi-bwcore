CXXFLAGS = -std=c++11 -g -Wall -DDEBUG
objects = util.o board.o bitptn.o

all: $(objects) main.o
	g++ $(objects) main.o -o bwcore1.5.exe

test: $(objects) test.o
	g++ $(objects) test.o -o test.exe

$(objects): util.h
test.o: board.h bitptn.h
main.o: board.h bitptn.h
bitptn.o:
board.o: bitptn.h
util.o:

.PHONY : clean
clean : 
	rm -f bwcore1.5.exe test.exe *.o
