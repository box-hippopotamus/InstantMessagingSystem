.PHONY:all
all: put get

put:put.cpp
	g++ -o $@ $^ -std=c++17 -letcd-cpp-api -lcpprest

get:get.cpp
	g++ -o $@ $^ -std=c++17 -letcd-cpp-api -lcpprest

.PHONY:clean
clean:
	rm -f put get