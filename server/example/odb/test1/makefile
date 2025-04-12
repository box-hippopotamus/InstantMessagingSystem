main: main.cpp
	c++ -o $@ $^ person-odb.cxx -lodb-mysql -lodb -lodb-boost

.PHONY:clean
clean:
	rm -f main