#include <string.h>
#include <arpa/inet.h>
#include "reqRep.h"
#include "users.h"
/*
*****************************************************************************************
 *	\note		D E F I N I T I O N   DES   M A C R O S
 */
/**
 *	\def		CHECK(sts, msg)
 *	\brief		Macro-fonction qui vérifie que sts est égal à -1 (cas d'erreur) 
 *				En cas d'erreur, il y a affichage du message adéquat et fin d'exécution  
 */
#define CHECK(sts, msg) if ((sts)==-1) {perror(msg); exit(-1);}
/**
 *	\def		CHECK_ZERO(sts, msg)
 *	\brief		Macro-fonction qui vérifie que sts est différent de 0 (cas d'erreur) 
 *				En cas d'erreur, il y a affichage du message adéquat et fin d'exécution  
 */
#define CHECK_NULL(sts, msg) if ((sts)==NULL) {perror(msg); exit(-1);}

users_t users;
int afficherUsers(char *cde) {
	printf("[%s] Liste des users [%d]\n", cde, users.nbUsers);	
	for (int i=0; i < users.nbUsers; i++)
			printf("\tUser [%d:%s], Etat [%c]\n",
				i,users.tab[i].name,users.tab[i].etat);
}

// recherche dans la liste des joueurs enregistrés
int trouverUser(name_t nom) {
	int i=0;
	for (i=0; i < MAX_USERS; i++)
		if (strcmp(nom, users.tab[i].name)==0) 
			return i;
	return -1;
}


// crée un user_t et l'enregistre dans les joueurs connectés
int creerUser(name_t nom, char * adrIP, short port) {
	if (users.nbUsers == MAX_USERS) 
		return -1;
	strncpy(users.tab[users.nbUsers].name, nom, MAX_NAME-1);

	if (strlen(nom)==MAX_NAME-1) 
		users.tab[users.nbUsers].name[MAX_NAME-1]='\0';

	
	users.tab[users.nbUsers].etat=ETAT_ONLINE;

	users.tab[users.nbUsers].adrIP = (char *)malloc(sizeof(char)*16); 
	strcpy(users.tab[users.nbUsers].adrIP, adrIP); 
	
	users.tab[users.nbUsers].port_srv_app = port; 
	
	users.nbUsers++;

	
	afficherUsers("créer");
	return users.nbUsers-1;
}

// enregistre une connection de joueur (met à jour / crée le joueur)
int identifierUser(char * userName, char * adrIP, short port) {
	requete_t req;
	int index = -1;
	if ((index=trouverUser(userName))==-1) {
		index=creerUser(userName, adrIP, port);
	}
	else { 
		users.tab[index].etat = ETAT_ONLINE; 

		users.tab[users.nbUsers].adrIP = (char *)malloc(sizeof(char)*16); 
		strcpy(users.tab[index].adrIP, adrIP); 

		users.tab[index].port_srv_app = port; 
	}

	//
	afficherUsers("identifier");
	
	return index;
} 

// déconnecte l'utilisateur et ferme la socket
void deconnecterUser(int indUser) {
	//printf(
	//	"Déconnexion : User [%s]\n",
	//	users.tab[indUser].name
	//);

	// CHECK(close(((socket_t *)users.tab[indUser].sDial)->fd),"--close()--");
	
	users.tab[indUser].etat = ETAT_OFFLINE;
	users.tab[indUser].port_srv_app = 0; 
	free(users.tab[indUser].adrIP); 

	//
	afficherUsers("déconnecter");
}

// change l'état d'un joueur
void modifierEtat(int indUser, etat_joueur_t etat) {
	users.tab[indUser].etat = etat;
	
	//
	afficherUsers("modifier");
	//return indDest;
}

char * nameUser(int indUser) {
	if (indUser==-1) 
		return NULL;
	else 
		return users.tab[indUser].name;
}

void lireUsers(void) {
	FILE *fp;
	CHECK_NULL(fp=fopen("users.dat", "r"), "--fopen()--");
	CHECK(users.nbUsers=fread(users.tab, sizeof(user_t), MAX_USERS, fp),"--fread()--");
	CHECK(fclose(fp),"--fclose()--");	

	afficherUsers("lecture");
}


void ecrireUsers(void) {
	FILE *fp;
	CHECK_NULL(fp=fopen("users.dat", "w"), "--fopen()--");
	CHECK(fwrite(users.tab, sizeof(user_t), users.nbUsers, fp),"--fread()--");
	CHECK(fclose(fp),"--fclose()--");	
	
	afficherUsers("ecriture");
}


void getListPseudoByState(etat_joueur_t etat, char * listePseudo){ 
	char * username = (char *)malloc(sizeof(char)*(MAX_NAME+1)); 
	int flag = 0; 
	strcpy(listePseudo, ""); 


	for (int i=0; i < MAX_USERS; i++){
		if (users.tab[i].etat==etat) {
			strcpy(username, nameUser(i)); 
			strcat(username, ":"); 
			strcat(listePseudo, username);
			flag = 1; 
		}
	}

	if( flag ){
		listePseudo[strlen(listePseudo)-1] = '\0';  
	}

	free(username); 
}

void getDetailsUser(int indUser, char * detailsUser){
	snprintf(detailsUser, TAILLE_OPT, "%c:%s:%hu", users.tab[indUser].etat, users.tab[indUser].adrIP, users.tab[indUser].port_srv_app); 
}