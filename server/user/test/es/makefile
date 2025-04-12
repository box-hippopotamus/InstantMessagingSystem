test:test.cpp
	g++ -o $@ $^ -std=c++17 -I../../../odb -lgflags -lfmt -lspdlog -lcpr -lelasticlient -ljsoncpp

.PHONY:clean
clean:
	rm -f test