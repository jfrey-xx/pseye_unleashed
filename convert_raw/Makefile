

all: opencv-test test


opencv-test: main.cpp v4ldevice.cpp pseye.o
	$(CXX) -Wall -g -O0 -o opencv-test main.cpp pseye.o v4ldevice.cpp $(shell pkg-config --libs-only-l opencv libv4l2)

pseye.o: pseye.cpp pseye.hpp

test: test.cpp pseye.o
	$(CXX) -Wall -g -O0 -o test test.cpp pseye.o $(shell pkg-config --libs-only-l opencv)



clean:
	$(RM) *.o
	$(RM) opencv-test
