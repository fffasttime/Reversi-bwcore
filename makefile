PYTHON3 = python
VPATH = src

ifdef def
	def = -D$(def)
endif

ifdef debug
	CXXFLAGS = -std=c++17 -g -Wall -DDEBUG $(def) -lpthread
	objects = util.o board.o evalptn.o search.o debugtree.o
else
	CXXFLAGS = -std=c++17 -Wall -O2 $(def) -lpthread
	objects = util.o board.o evalptn.o search.o
endif

ifndef o
	output = $@.exe
else
	output = $(o).exe
endif

all: $(objects) cui.o
	$(CXX) $(objects) cui.o src/main.cpp $(CXXFLAGS) -o bwcore1.5.exe

online: $(objects)
	$(CXX) $(objects) src/online.cpp $(CXXFLAGS) -o $(output)

test: $(objects)
	$(CXX) $(objects) src/test.cpp $(CXXFLAGS) -o $(output)

gendata: $(objects)
	$(CXX) $(objects) src/gendata.cpp $(CXXFLAGS) -o $(output)

libboardfeature: 
	$(CXX) -fPIC -shared src/boardfeature.cpp $(CXXFLAGS) -o $@.so

judger: util.o board.o
	$(CXX) util.o board.o src/judger.cpp $(CXXFLAGS) -o $(output)

linreg: util.o board.o evalptn.o
	$(CXX) util.o board.o evalptn.o src/linreg.cpp $(CXXFLAGS) -o $(output)

$(objects): util.h
cui.o: board.h search.h evalptn.h
evalptn.o: board.h evalptn.h
search.o: board.h search.h debugtree.h evalptn.h
debugtree.o: board.h
board.o:  board.h
util.o:

bwcore14:
	$(CXX) -O2 tools/bwcore_online1.4.cpp -o bwcore_online1.4.exe

.PHONY: botzone
botzone:
	$(PYTHON3) tools/merge.py
	$(CXX) $(CXXFLAGS) botzone.cpp -o botzone.exe

.PHONY: clean
clean: 
	$(RM) *.o bwcore1.5.exe online.exe test.exe gendata.exe judger.exe linreg.exe debugtree.html
