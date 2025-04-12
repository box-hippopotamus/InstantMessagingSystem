.PHONY:all
all: test

test:test.cpp
	g++ -o $@ $^ -std=c++17 -lgflags

.PHONY:clean
clean:
	rm -f test