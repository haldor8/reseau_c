#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>

#define TAILLE_BUFFER 1024

void afficher_message_serveur(char* message, int descripteur){
    printf("Message recu cote serveur du socket %d : [%s]\n", descripteur, message);
}

long taille_fichier(FILE* fichier){
    // Retourne la taille du fichier donné
    long position_actuelle = ftell(fichier);
    fseek(fichier, 0, SEEK_END);
    long taille = ftell(fichier);
    fseek(fichier, position_actuelle, SEEK_SET);
    return taille;
}

void envoyer_extraits_fichier(char* nom_fichier, int debut, int fin, int descripteur){
    FILE* fichier;
    // On réinitialise manuellement tous les buffers pour éviter les segmentation fault
    char ligne[TAILLE_BUFFER] = {0}; // La ligne courante du fichier lue
    char message[TAILLE_BUFFER] = ""; // Le message à envoyer

    fichier = fopen(nom_fichier, "r");
    if(fichier == NULL){
        strcpy(message, "Impossible d'ouvrir le fichier.");
    }else{
        // Si on souhaite prendre le fichier au complet
        if(debut == 1 && fin == -1){
            if(taille_fichier(fichier) > TAILLE_BUFFER){
                strcpy(message, "Fichier trop grand. Buffer overflow."); // Pour éviter que le programme plante
            }else{
                while(fgets(ligne, sizeof(ligne), fichier) != NULL){
                    strcat(message, ligne);
                }
            }
        }else{
            int cpt_ligne = 1;
            while(fgets(ligne, sizeof(ligne), fichier) != NULL){
                if(cpt_ligne >= debut && cpt_ligne <= fin){
                    if(strlen(ligne) + strlen(message) > TAILLE_BUFFER){
                        // Pour éviter que le programme plante
                        strcpy(message, "Fichier trop grand. Buffer overflow.");
                        break;
                    }else{
                        strcat(message, ligne);
                    }
                }
                cpt_ligne++;
            }
        }
    }

    write(descripteur, message, strlen(message)); // On envoie l'ensemble des lignes prises en compte
    fclose(fichier);
}

void gerer_commandes(int descripteur){
    char buffer[TAILLE_BUFFER] = {0};
    int obj_recu;
    unsigned int debut, fin;
    char nom_fichier[TAILLE_BUFFER] = {0};

    obj_recu = read(descripteur, buffer, TAILLE_BUFFER);

    buffer[obj_recu] = '\0'; // Pour s'assurer que le message est bien formaté
    afficher_message_serveur(buffer, descripteur);

    // Si l'utilisateur a décidé de quitter le programme alors on ferme l'écouteur
    if(!strcmp(buffer, "exit")){
        // Le client a signalé au serveur qu'il sortait du programme, donc on le déconnecte
        printf("Fermeture de l'ecouteur client numero %d.\n", descripteur);
        close(descripteur);
        return;
    }

    int elt_lus = sscanf(buffer, "lire %s %u %u", nom_fichier, &debut, &fin); // Si un élément de lu alors ça veut dire qu'on a 
    if(elt_lus == 3){
        envoyer_extraits_fichier(nom_fichier, debut, fin, descripteur); // Si on a spécifié 3 arguments alors on envoie tous les arguments
    }else if(elt_lus == 2){
        envoyer_extraits_fichier(nom_fichier, debut, -1, descripteur); // On envoie que le début et le nom du fichier si 2
    }else if(elt_lus == 1) {
        envoyer_extraits_fichier(nom_fichier, 1, -1, descripteur); // On envoie que le nom du fichier si 1 un seul spécifié
    }else{
        char* message_retour = "Commande inconnue ou mal formatee, utilisez \"help\" pour voir la liste des commandes";
        write(descripteur, message_retour, strlen(message_retour));
    }

    gerer_commandes(descripteur); // On appelle récursivement le programme jusqu'à arrêt via exit
}

void *gerer_client(void* arg){
    int descripteur = *(int*) arg;
    free(arg);

    printf("Client connecte, socket : %d\n", descripteur);
    gerer_commandes(descripteur); // On gère les commandes jusqu'à ce que le client se déconnecte
    return NULL;
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

    // attente de connexion
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
        pthread_create(&ecouteur_client, NULL, &gerer_client, (void*)arg); // On lance les écouteurs en thread.
    }

    close(ma_socket);

    return 0;
}