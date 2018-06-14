Parse: main.o Parsing.o 
	g++ -std=c++11 -o Parse main.o Parsing.o

main.o: main.cpp Parsing.h
	g++ -std=c++11 -c main.cpp 

Parsing.o: Parsing.cpp Parsing.h
	g++ -std=c++11 -c Parsing.cpp

clean:
	rm Parse main.o Parsing.o 