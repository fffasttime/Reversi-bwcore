CXXFLAGS = -std=c++11 -g -Wall -DDEBUG
objects = main.o util.o board.o bitptn.o

all: $(objects)
	g++ $(objects) -o bwcore1.5.exe

$(objects): util.h

main.o:
board.o: bitptn.h
bitptn.o:
util.o:

.PHONY : clean
clean : 
	rm -f bwcore1.5.exe *.o
