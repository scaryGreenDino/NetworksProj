all: server client clean

server: server.o checksum.o
	g++ -o server server.o checksum.o

client: client.o checksum.o
	g++ -o client client.o checksum.o

server.o:
	g++ -c server.cpp -std=c++11

client.o:
	g++ -c client.cpp -std=c++11

checksum.o:
	g++ -c checksum.cpp -std=c++11 

clean:
	rm -f *~
	rm -f *.o
