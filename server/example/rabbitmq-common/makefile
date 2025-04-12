# 链接库
LIBS = -lamqpcpp -lev -lgflags -lfmt -lspdlog

# 目标
TARGETS = publish consume

.PHONY: all
all: $(TARGETS)

publish: publish.cpp
	g++ -o $@ $^ -std=c++17 $(LIBS)

consume: consume.cpp
	g++ -o $@ $^ -std=c++17 $(LIBS)

.PHONY: clean
clean:
	rm -f $(TARGETS)