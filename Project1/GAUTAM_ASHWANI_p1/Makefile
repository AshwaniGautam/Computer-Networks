all: server 
server: server.cpp
	g++ -std=c++11 server.cpp -o http-server
	g++ -o server server.o

clean:
	rm http-server
