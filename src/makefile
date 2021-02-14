ifdef release
	CXXFLAGS = -std=c++17 -Wall -O2
else
	CXXFLAGS = -std=c++17 -g -Wall -DDEBUG
endif

objects = util.o board.o evalptn.o search.o debugtree.o

all: $(objects) main.o cui.o
	g++ $(CXXFLAGS) $(objects) cui.o main.o -o bwcore1.5.exe

online: $(objects) online.o
	g++ $(CXXFLAGS) $(objects) online.o -o bwcore_online1.5.exe

test: $(objects) test.o
	g++ $(CXXFLAGS) $(objects) test.o -o test.exe

$(objects): util.h
test.o: board.h evalptn.h search.h debugtree.h
main.o: board.h cui.h
online.o: board.h search.h evalptn.h
cui.o: board.h search.h evalptn.h
evalptn.o: board.h evalptn.h
search.o: board.h search.h debugtree.h evalptn.h
debugtree.o: board.h
board.o:  board.h
util.o:

.PHONY: clean
clean: 
	rm -f *.exe *.o debugtree.html
