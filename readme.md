# Lancer le programme
Pour lancer le programme il suffit de lancer client.exe (possible d'en lancer plusieurs) et serveur.exe (qu'un seul à la fois).
Il faut les lancer manuellement dans différents terminaux.

# Les commandes
`help` donne la liste des commandes utilisables.
`exit` ferme le client et signale au serveur de fermer son écouteur.
`lire` permet de spécifier un fichier à lire.

# Le makefile
Pour le makefile, il y a simplement deux exécutables à créer : `serveur` et `client`.
Si vous souhaitez utiliser un gcc particulier, il suffit de modifier la variable `$(CC)` du makefile par le chemin vers le compilateur.

Pour nettoyer les fichiers il y a plusieurs commandes :
- `clean` : si on est sur le même terminal que l'OS.
- `man_clean_w` : pour utiliser les fonctions de windows s'il est émulé sur un autre OS.
- `man_clean_l` : pour utiliser les fonctions de linux s'il est émulé sur un autre OS.