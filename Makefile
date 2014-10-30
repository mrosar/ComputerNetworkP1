all : sender receiver
sender : sender.o
	gcc -g sender.o -o sender
sender.o : sender.c
	gcc -g -c sender.c -o sender.o

receiver : receiver.o
	gcc -g receiver.o -o receiver
receiver.o : receiver.c
	gcc -g -c receiver.c -o receiver.o
