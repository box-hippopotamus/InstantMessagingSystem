# 链接库
LIBS = -lalibabacloud-sdk-core -l:libjsoncpp.so.25 -lcurl

# 目标
TARGETS = test

.PHONY: all
all: $(TARGETS)

test: test.cpp
	g++ -g -o $@ $^ -std=c++17 $(LIBS)

.PHONY: clean
clean:
	rm -f $(TARGETS)