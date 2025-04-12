.PHONY:all
all: test main

test:test.cpp
	g++ -o $@ $^ -std=c++17 -lfmt -lspdlog

main:main.cpp
	g++ -o $@ $^ -std=c++17 -lfmt -lspdlog -lgflags

.PHONY:clean
clean:
	rm -f test main