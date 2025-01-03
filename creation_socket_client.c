#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TAILLE_BUFFER 1024

void afficher_message_client(char* message, int taille_message){
    // Réutilisation du script fourni
    printf("Reception client : %d [", taille_message);
    for (int i = 0; i < taille_message; i++)
    {
        printf("%c", message[i]);
    }
    printf("]\n");
}

void ecouter_input(int socket){
    // Echange d'information
    char commande[TAILLE_BUFFER];
    printf("Veuillez entrer une commande : \n");
    fgets(commande, TAILLE_BUFFER, stdin);
    commande[strcspn(commande, "\n")] = '\0'; // On remplace le \n par un \0 pour que le message puisse être bien lu

    if(!strcmp(commande, "exit")){
        write(socket, "exit", 4);
        return;
    }else if(!strcmp(commande, "help")){
        printf("Liste des commandes :\n");
        printf("Pour sortir du programme : \"exit\"\n");
        printf("Pour lire un fichier complet : \"lire <chemin/nom_fichier.extension> options : <position_debut> <position_fin>\"");
    }else{
        write(socket, commande, strlen(commande)); // On envoie le message
        char buffer[TAILLE_BUFFER] = {0};
        int mon_nb_lus;
        mon_nb_lus = read(socket, buffer, TAILLE_BUFFER);
        buffer[mon_nb_lus] = '\0';
        afficher_message_client(buffer, mon_nb_lus);
    }

    ecouter_input(socket); // On écoute jusqu'à sortie du programme
}

int main()
{
    /*
        socket -> création de la prise
            #include sys/socket.h
            int socket(int domaine, int type, int protocole);
                domaine : PF_INET (==> IPv4) ou PF_INET6 (==> IPv6)
                type : SOCK_STREAM (~ TCP) ou SOCK_DGRAM (~ UDP) ...
                protocole : famille de protocole ==> 0

                retourne le descripteur

        connect -> attente de connection
            #include sys/socket.h
            int accept(int socket, struct sockaddr* adresse, socklen_t longueur);
                socket : la prise définie + haut
                adresse : inforamtion sur le client (adresse)
                longueur : inforamtion sur le client (taille de l'adresse du client)

                retourne un descripteur de d'entrée sortie

        send/receve -> echange
        close -> fermeture

    */

    // Creation de la socket
    int ma_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (ma_socket == -1)
    {
        fprintf(stderr, "pb de création de socket\n");
        exit(EXIT_FAILURE);
    }
    // Resolution de nom

    // connection au serveur
    struct sockaddr_in mon_adresse_serveur;
    memset(&mon_adresse_serveur, 0, sizeof(mon_adresse_serveur));
    mon_adresse_serveur.sin_family = AF_INET;
    mon_adresse_serveur.sin_port = htons(61234);
    mon_adresse_serveur.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(ma_socket, (struct sockaddr *)&mon_adresse_serveur, sizeof(struct sockaddr_in)))
    {
        fprintf(stderr, "Serveur non trouve, delais d'attente de connexion depasse.\n");
        close(ma_socket);
        exit(EXIT_FAILURE);
    }

    sleep(1);
    ecouter_input(ma_socket);

    close(ma_socket);

    return 0;
}