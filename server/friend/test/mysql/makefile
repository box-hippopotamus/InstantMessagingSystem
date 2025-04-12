CFLAGS = -I/usr/include/mysql -I../../../odb/ -I../../../common -I./ 
test : test.cpp relation-odb.cxx friend_apply-odb.cxx chat_session-odb.cxx session_member-odb.cxx
	g++ -std=c++17 $^ -o $@ -lodb-mysql $(CFLAGS) -lodb -lodb-boost -lfmt -lspdlog -lgflags