.PHONY:all
all: test

test:test.cpp
	g++ -o $@ $^ -std=c++17 -lpthread -lboost_system

.PHONY:clean
clean:
	rm -f test