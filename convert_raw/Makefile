
all: opencv-test test pseye2loopback

opencv-test: main.cpp v4ldevice.cpp pseye.o
	$(CXX) -Wall -g -O0 -o opencv-test main.cpp pseye.o $(shell pkg-config --libs-only-l opencv libv4l2)

pseye2loopback: pseye2loopback.cpp pseye.o outloop.o
	$(CXX) -Wall -g -O0 -o pseye2loopback pseye2loopback.cpp pseye.o outloop.o $(shell pkg-config --libs-only-l opencv libv4l2)

pseye.o: pseye.cpp pseye.hpp

outloop.o: outloop.cpp outloop.hpp

test: test.cpp pseye.o
	$(CXX) -Wall -g -O0 -o test test.cpp pseye.o $(shell pkg-config --libs-only-l opencv)

clean:
	$(RM) *.o
	$(RM) opencv-test test pseye2loopback
