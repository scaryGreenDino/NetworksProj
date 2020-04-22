all: sender reciever clean

sender: sender.o 
	g++ -o sender sender.o 

reciever: reciever.o 
	g++ -o reciever reciever.o 

sender.o:
	g++ -c sender.cpp -std=c++11 

reciever.o:
	g++ -c reciever.cpp 

clean:
	rm -f *~
	rm -f *.ovim 