.PHONY:all
all: test

test:test.cpp
	g++ -o $@ $^ -std=c++17 -lcpr -lelasticlient

.PHONY:clean
clean:
	rm -f test