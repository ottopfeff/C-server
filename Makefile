all:
	gcc -c list.c
	gcc client.c -o client.exe -lws2_32 
	gcc server.c list.o -o server.exe -lws2_32 