.PHONY:all
all: server client

server:server.cpp test.pb.cc
	g++ -o $@ $^ -std=c++17 -lbrpc -lleveldb -lgflags -lssl -lcrypto -lprotobuf

client:client.cpp test.pb.cc
	g++ -o $@ $^ -std=c++17 -lbrpc -lleveldb -lgflags -lssl -lcrypto -lprotobuf

.PHONY:clean
clean:
	rm -f server client