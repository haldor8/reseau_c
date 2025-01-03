CC = gcc # C:/cygwin64/bin/gcc
CFLAGS = -Wall -Wextra -std=c11 

serveur : serveur.o
	$(CC) $(CFLAGS) -o serveur.exe creation_socket_serveur.o

serveur.o : creation_socket_serveur.c
	$(CC) $(CFLAGS) -c creation_socket_serveur.c -lpthread

client : client.o
	$(CC) $(CFLAGS) -o client.exe creation_socket_client.o

client.o : creation_socket_client.c
	$(CC) $(CFLAGS) -c creation_socket_client.c

# Ne fonctionne que pour l'OS lui-même, si on utilise Cygwin sur Windows, il faut utiliser "man_clean_l"
ifeq ($(OS),Windows_NT)
    RM = del /Q
else
    RM = rm -f
endif
# Règle pour nettoyer les fichiers générés
clean:
	$(RM) *.o
	$(RM) *.exe

# Règle pour nettoyer manuellement
man_clean_w:
	del *.o
	del *.exe

man_clean_l:
	RM -f *.o
	RM -f *.exe