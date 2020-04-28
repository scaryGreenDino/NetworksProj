all: sender reciever clean

sender: sender.o convert.o
	g++ -o sender sender.o convert.o

reciever: reciever.o convert.o
	g++ -o reciever reciever.o convert.o

sender.o:
	g++ -c sender.cpp -std=c++11 

reciever.o:
	g++ -c reciever.cpp 

convert.o:
	g++ -c convert.cpp

clean:
	rm -f *~
	rm -f *.o