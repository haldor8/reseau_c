#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <windows.h>
#include <pthread.h>

#include "creation_socket_client.c"
#include "creation_socket_serveur.c"

int main()
{
    pthread_t client;
    pthread_t serveur;

    printf("Lancement des threads\n");
    pthread_create(&serveur, NULL, *creer_serveur, NULL);
    pthread_create(&client, NULL, *creer_client, NULL);

    sleep(10);

    printf("Jointure des threads\n");
    pthread_join(client, NULL);
    pthread_join(serveur, NULL);
    
    return 0;
}