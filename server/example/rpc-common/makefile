.PHONY:all
all: es

es:es.cpp
	g++ -o $@ $^ -std=c++17 -lelasticlient -lspdlog -lfmt -lgflags -ljsoncpp

.PHONY:clean
clean:
	rm -f es

# ========================== 测试 es ==========================
# .PHONY:all
# all: es

# es:es.cpp
# 	g++ -o $@ $^ -std=c++17 -lelasticlient -lspdlog -lfmt -lgflags -ljsoncpp

# .PHONY:clean
# clean:
# 	rm -f es

# ========================== 测试 brpc ==========================
# .PHONY:all
# all: discovery registry

# discovery:discovery.cpp test.pb.cc
# 	g++ -o $@ $^ -std=c++17 -lprotobuf -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags -lbrpc -lleveldb -lssl -lcrypto

# registry:registry.cpp test.pb.cc
# 	g++ -o $@ $^ -std=c++17 -lprotobuf -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags -lbrpc -lleveldb -lssl -lcrypto 

# .PHONY:clean
# clean:
# 	rm -f discovery registry