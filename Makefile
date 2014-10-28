all : sender receiver
sender : sender.o
	gcc sender.o -o sender
sender.o : sender.c
	gcc -c sender.c -o sender.o

receiver : receiver.o
	gcc receiver.o -o receiver
receiver.o : receiver.c
	gcc -c receiver.c -o receiver.o