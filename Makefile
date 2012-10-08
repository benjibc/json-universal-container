CXX := g++
all:
	g++ -Wall -o variant -I ./boost_file/ jsonUnitTest.cpp -std=c++0x -fmax-errors=3 -ggdb 
