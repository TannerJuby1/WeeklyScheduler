
CXXFLAGS = -g -std=c++11 -lpthread
OBJS = server.o client.o server client

all:
	g++ server.cpp -o server
	g++ client.cpp -o client

server: server.cpp
	g++ server.cpp -o server -c $(CXXFLAGS)

client: client.cpp
	g++ client.cpp -o client -c $(CXXFLAGS)

clean:
	rm -f $(OBJS)
