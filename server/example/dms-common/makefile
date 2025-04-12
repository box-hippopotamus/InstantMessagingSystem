# 链接库
LIBS = -lfmt -lgflags -lcurl -lalibabacloud-sdk-core -l:libjsoncpp.so.25

# 目标
TARGETS = test

.PHONY: all
all: $(TARGETS)

test: test.cpp
	g++ -g -o $@ $^ -std=c++17 $(LIBS)

.PHONY: clean
clean:
	rm -f $(TARGETS)