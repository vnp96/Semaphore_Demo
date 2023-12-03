test:	main.o
	g++ -w -std=c++23 main.o -o test

main.o: main.cpp
	g++ -w -std=c++23 -c main.cpp

clean:
	rm -f test
	rm -f *.o *.gch

