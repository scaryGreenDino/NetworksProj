all: server client clean

server: server.o 
	g++ -o server server.o

client: client.o 
	g++ -o client client.o 

server.o:
	g++ -c server.cpp -std=c++11

client.o:
	g++ -c client.cpp -std=c++11

clean:
	rm -f *~
	rm -f *.o
