simplegrep: simplegrep.cpp ThreadPool.cpp
	g++ -pthread -lstdc++fs -std=c++17 -Wall   ThreadPool.cpp simplegrep.cpp   -o simplegrep -lstdc++fs
clean:
	rm simplegrep
	rm *.txt
	rm *.log