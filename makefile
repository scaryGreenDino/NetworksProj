all: kdc sender reciever clean

kdc: kdc.o blowfish.o
	g++ -o kdc kdc.o blowfish.o

sender: sender.o blowfish.o function.o convert.o
	g++ -o sender sender.o blowfish.o function.o convert.o

reciever: reciever.o blowfish.o function.o convert.o
	g++ -o reciever reciever.o blowfish.o function.o convert.o

kdc.o:
	g++ -c kdc.cpp -std=c++11 

sender.o:
	g++ -c sender.cpp -std=c++11 

reciever.o:
	g++ -c reciever.cpp 

blowfish.o:
	g++ -c blowfish.cpp -std=c++11 

function.o:
	g++ -c function.cpp

convert.o:
	g++ -c convert.cpp

clean:
	rm -f *~
	rm -f *.o