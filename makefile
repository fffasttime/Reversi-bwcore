PYTHON3 = python

ifdef def
	def = -D$(def)
endif

ifdef debug
	CXXFLAGS = -std=c++17 -g -Wall -DDEBUG $(def)
	objects = util.o board.o evalptn.o search.o debugtree.o
else
	CXXFLAGS = -std=c++17 -Wall -O2 $(def)
	objects = util.o board.o evalptn.o search.o
endif

ifndef o
	output = $@.exe
else
	output = $(o).exe
endif

all: $(objects) cui.o
	$(CXX) $(CXXFLAGS) $(objects) cui.o src/main.cpp -o bwcore1.5.exe

online: $(objects)
	$(CXX) $(CXXFLAGS) $(objects) src/online.cpp -o $(output)

test: $(objects)
	$(CXX) $(CXXFLAGS) $(objects) src/test.cpp -o $(output)

gendata: $(objects)
	$(CXX) $(CXXFLAGS) $(objects) src/gendata.cpp -o $(output)

judger: util.o board.o
	$(CXX) $(CXXFLAGS) util.o board.o src/judger.cpp -o $(output)

linreg: util.o board.o evalptn.o
	$(CXX) $(CXXFLAGS) util.o board.o evalptn.o src/linreg.cpp -o $(output)

$(objects): util.h
cui.o: board.h search.h evalptn.h
evalptn.o: board.h evalptn.h
search.o: board.h search.h debugtree.h evalptn.h
debugtree.o: board.h
board.o:  board.h
util.o:

VPATH = src

bwcore14:
	$(CXX) -O2 tools/bwcore_online1.4.cpp -o bwcore_online1.4.exe

.PHONY: botzone
botzone:
	$(PYTHON3) tools/merge.py
	$(CXX) $(CXXFLAGS) data/botzone.cpp -o botzone.exe

.PHONY: clean
clean: 
	$(RM) *.o bwcore1.5.exe online.exe test.exe gendata.exe judger.exe linreg.exe debugtree.html
