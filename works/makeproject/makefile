game : ../source/game.cpp ../source/ThreadSafeQueue.cpp ../source/dila.hpp ../source/simulator.hpp
	g++ ../source/game.cpp ../source/ThreadSafeQueue.cpp ../source/dila.hpp ../source/simulator.hpp -std=c++0x -o ../target/game -g -lpthread
	cp ../source/data/holdRank.data ../target
all: game
clean:
	rm -f ../target/game

