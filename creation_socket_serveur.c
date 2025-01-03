#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TAILLE_BUFFER 256

void afficher_message_serveur(char* message){
    printf("Message recu cote serveur : [%s]\n", message);
}

void gerer_commandes(int descripteur){
    char buffer[TAILLE_BUFFER];
    int obj_recu;
    long debut, fin;
    char nom_fichier[TAILLE_BUFFER];

    obj_recu = read(descripteur, buffer, TAILLE_BUFFER);

    afficher_message_serveur(buffer);

    // Si l'utilisateur a décidé de quitter le programme alors on ferme l'écouteur
    if(!strcmp(buffer, "exit")){
        // shutdown(mon_descripteur, SHUT_RDWR);
        printf("Fermeture de l'ecouteur client");
        close(descripteur);
        return;
    }

    if(sscanf(buffer, "lire %s %ld %ld", nom_fichier, &debut, &fin) != 3){
        char* message_retour = "Commande mal formulee, format : \"lire <nom_fichier> <position_debut> <position_fin>\"";
        write(descripteur, message_retour, strlen(message_retour));
    }else{
        
    }

    gerer_commandes(descripteur);
}

void *gerer_client(void* arg){
    int descripteur = *(int*) arg;
    free(arg);

    printf("Client connecte, socket : %d\n", descripteur);
    gerer_commandes(descripteur); // On gère les commandes jusqu'à ce que le client se déconnecte
    return NULL;
}

void *creer_serveur(void *arg)
{
    arg = arg;
    /*
        socket -> création de la prise
            #include sys/socket.h
            int socket(int domaine, int type, int protocole);
                domaine : PF_INET (==> IPv4) ou PF_INET6 (==> IPv6)
                type : SOCK_STREAM (~ TCP) ou SOCK_DGRAM (~ UDP) ...
                protocole : famille de protocole ==> 0

                retourne le descripteur

        bind -> branchement au réseau
            #include sys/socket.h
            int bind(int socket, const struct sockaddr* adresse, socklen_t longueur);
                socket : la prise créée au dessus
                adresse : l'adresse d'une sockaddr (selon IPv4 ou IPv6 => une sockaddr_in ou un sockaddr_in6)
                longueur : la taille de l'adresse (un sizeof fera l'affaire)

                retourne 0 => pb

        listen -> calibrage
            #include sys/socket.h
            int listen(int socket, int attente);
                socket : la prise définie + haut
                attente : le nombre d'appels qui peuvent être traité (1 = coup de fil, 2 = 1 traité + 1 en attente, 3 = 1 traité + 2 en attente, 0 = le système gère "au mieux")

                retourne différent de 0 => pb

        accept -> attente de connection
            #include sys/socket.h
            int accept(int socket, struct sockaddr* adresse, socklen_t longueur);
                socket : la prise définie + haut
                adresse : inforamtion sur le client (adresse)
                longueur : inforamtion sur le client (taille de l'adresse du client)

                retourne un descripteur de d'entrée sortie

        On peut dégrader l'une ou l'autre des sens d'émission avec
            shutdown(int socket, int how)
                socket : la prise définie + haut
                how : SHUT_WR ou SHUT_RD ou SHUT_RDWR


        write/read -> echange
        close -> fermeture

        possibilité de fermeture partielle
    */

    // Creation de la socket
    int ma_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (ma_socket == -1)
    {
        fprintf(stderr, "pb de création de socket\n");
        exit(EXIT_FAILURE);
    }

    // branchement au reseau
    struct sockaddr_in ma_socket_ipv4;
    memset(&ma_socket_ipv4, 0, sizeof(ma_socket_ipv4));
    ma_socket_ipv4.sin_family = AF_INET;
    ma_socket_ipv4.sin_port = htons(61234);
    ma_socket_ipv4.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(ma_socket, (struct sockaddr *)&ma_socket_ipv4, sizeof(ma_socket_ipv4)))
    {
        fprintf(stderr, "pb de branchement de socket\n");
        close(ma_socket);
        exit(EXIT_FAILURE);
    }
    printf("mon adresse en chaine %s => %x %d\n", inet_ntoa(ma_socket_ipv4.sin_addr), ma_socket_ipv4.sin_addr.s_addr, INADDR_ANY);

    // calibrage
    if (listen(ma_socket, 0))
    {
        fprintf(stderr, "pb de calibrage de socket\n");
        close(ma_socket);
        exit(EXIT_FAILURE);
    }

    // attente de connection
    while(1){
        int mon_descripteur;
        struct sockaddr_in6 mon_adresse_client;
        int ma_taille_socket_client;
        mon_descripteur = accept(ma_socket, (struct sockaddr *)&mon_adresse_client, &ma_taille_socket_client);
        if (mon_descripteur == -1)
        {
            fprintf(stderr, "pb de ecoute\n");
            close(ma_socket);
            exit(EXIT_FAILURE);
        }
        if (mon_adresse_client.sin6_family == AF_INET6)
        {
            char mon_texte_adresse_client[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(mon_adresse_client.sin6_addr), mon_texte_adresse_client, INET6_ADDRSTRLEN);
            printf("Connexion avec %s %d\n", mon_texte_adresse_client, mon_adresse_client.sin6_port);
        }
        else
        {
            printf("Connexion avec %s %d\n", inet_ntoa(((struct sockaddr_in *)&mon_adresse_client)->sin_addr), ((struct sockaddr_in *)&mon_adresse_client)->sin_port);
        }
        pthread_t ecouteur_client;
        int* arg = (int*)malloc(sizeof(int));
        *arg = mon_descripteur;
        pthread_create(&ecouteur_client, NULL, &gerer_client, (void*)arg);
    }

    close(ma_socket);

    return 0;
}