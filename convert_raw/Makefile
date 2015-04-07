

all: opencv-test


opencv-test: main.cpp v4ldevice.cpp
	$(CXX) -Wall -g -O0 -o opencv-test main.cpp v4ldevice.cpp $(shell pkg-config --libs-only-l opencv libv4l2)

clean:
	$(RM) *.o
	$(RM) opencv-test
