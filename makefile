CXX = g++
CXXFLAGS = -Wall -std=c++17 -O3

default: db_dir game.o sumgame.o cache.o main.o
	$(CXX) $(CXXFLAGS) game.o sumgame.o cache.o main.o -o solver_main

db_dir:
	@if [ ! -d "./db/" ]; then\
		echo "db downloading";\
		curl -s https://webdocs.cs.ualberta.ca/~du2/nogo/db.tgz -o db.tgz;\
		tar -xf db.tgz;\
	else\
		echo "db downloaded";\
	fi

check_database: check_database.cpp board.hpp
	$(CXX) $(CXXFLAGS) check_database.cpp -o check_database

main.o: main.cpp cache.hpp zobrist_hash.hpp game.hpp color.hpp board.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

cache.o: cache.cpp cache.hpp color.hpp board.hpp game.hpp
	$(CXX) $(CXXFLAGS) -c cache.cpp

sumgame.o: sumgame.cpp sumgame.hpp color.hpp board.hpp game.hpp zobrist_hash.hpp cache.hpp
	$(CXX) $(CXXFLAGS) -c sumgame.cpp

game.o: game.cpp game.hpp color.hpp board.hpp sumgame.hpp cache.hpp
	$(CXX) $(CXXFLAGS) -c game.cpp

clean:
	rm -rf db.tgz *.o solver_main check_database
