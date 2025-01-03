CC = C:/cygwin64/bin/gcc
CFLAGS = -Wall -Wextra -std=c11 

serveur : serveur.o
	$(CC) $(CFLAGS) -o serveur.exe creation_socket_serveur.o

serveur.o : creation_socket_serveur.c
	$(CC) $(CFLAGS) -c creation_socket_serveur.c

client : client.o
	$(CC) $(CFLAGS) -o client.exe creation_socket_client.o

client.o : creation_socket_client.c
	$(CC) $(CFLAGS) -c creation_socket_client.c