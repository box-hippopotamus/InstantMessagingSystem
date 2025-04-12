.PHONY:all
all:publish consume

publish:publish.cpp
	g++ -o $@ $^ -std=c++17 -lamqpcpp -lev

consume:consume.cpp
	g++ -o $@ $^ -std=c++17 -lamqpcpp -lev

.PHONY:clean
clean:
	rm -f publish consume