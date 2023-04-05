simplegrep: simplegrep.cpp
	g++ -lstdc++fs -std=c++17 -Wall    simplegrep.cpp   -o simplegrep -lstdc++fs
clean:
	rm simplegrep