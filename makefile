include src/makefile
VPATH = src

.PHONY: botzone
botzone:
	python tools/merge.py
	g++ $(CXXFLAGS) data/botzone.cpp -o botzone.exe
