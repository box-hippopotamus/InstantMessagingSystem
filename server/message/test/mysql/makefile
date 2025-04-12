CFLAGS = -I/usr/include/mysql -I../../../odb/ -I../../../common -I./ 
test : test.cpp message-odb.cxx
	g++ -std=c++17 $^ -o $@   -lodb-mysql $(CFLAGS) -lodb -lodb-boost -lfmt -lspdlog -lgflags