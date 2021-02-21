ifdef release
	CXXFLAGS = -std=c++17 -Wall -O2 $(def)
	objects = util.o board.o evalptn.o search.o
else
	CXXFLAGS = -std=c++17 -g -Wall -DDEBUG $(def)
	objects = util.o board.o evalptn.o search.o debugtree.o
endif

all: $(objects) cui.o
	g++ $(CXXFLAGS) $(objects) cui.o src/main.cpp -o bwcore1.5.exe

online: $(objects)
	g++ $(CXXFLAGS) $(objects) src/online.cpp -o bwcore_online1.5.exe

test: $(objects)
	g++ $(CXXFLAGS) $(objects) src/test.cpp -o test.exe

gendata: $(objects)
	g++ $(CXXFLAGS) $(objects) src/gendata.cpp -o gendata.exe

judger: util.o board.o
	g++ $(CXXFLAGS) util.o board.o src/judger.cpp -o judger.exe

$(objects): util.h
cui.o: board.h search.h evalptn.h
evalptn.o: board.h evalptn.h
search.o: board.h search.h debugtree.h evalptn.h
debugtree.o: board.h
board.o:  board.h
util.o:

VPATH = src

bwcore14:
	g++ -O2 tools/bwcore_online1.4.cpp -o bwcore_online1.4.exe

.PHONY: botzone
botzone:
	python tools/merge.py
	g++ $(CXXFLAGS) data/botzone.cpp -o botzone.exe

.PHONY: clean
clean: 
	rm -f *.exe *.o debugtree.html
