#ifndef USERS_H
#define USERS_H
#include "session.h"

#define MAX_NAME 64
#define MAX_USERS	16

typedef char name_t[MAX_NAME];

typedef struct {
	name_t name;
	char * adrIP; 
	short port_srv_app; 
	char etat;
} user_t;



typedef struct {
	user_t tab[MAX_USERS];
	int nbUsers;
} users_t;

typedef enum { 
	ETAT_OFFLINE='X',
	ETAT_ONLINE='O',
	ETAT_HOST='H',
	ETAT_FULL='F'
} etat_joueur_t; 




int trouverUser(char *nom);  // recherche dans la liste des joueurs enregistrés
int creerUser(name_t nom, char * adrIP, short port) ;  // crée un user_t et l'enregistre dans les joueurs connectés
int identifierUser(char * userName, char * adrIP, short port);  // enregistre une connection de joueur (met à jour / crée le joueur)
void deconnecterUser(int indUser);  // déconnecte l'utilisateur et ferme la socket
void modifierEtat(int indUser, etat_joueur_t etat);  // change l'état d'un joueur
char * nameUser(int indUser);
void lireUsers(void);
void ecrireUsers(void);
void getListPseudoByState(etat_joueur_t etat, char * listePseudo); // récupère la liste des joueurs selon leur état
#endif /* USERS_H */ 
