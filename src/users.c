/**
 * \file users.c
 * \brief Gestion des utilisateurs (création, identification, état, persistance)
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

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

/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S   G L O B A L E S
 */
/**
 * \var users
 * \brief Structure globale contenant la liste des utilisateurs enregistrés
 */
users_t users;

/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

 /**
 * \fn int afficherUsers(char *cde)
 * \brief Affiche la liste des utilisateurs avec leur état
 * \param cde Chaîne indiquant le contexte d'appel (ex: "créer", "identifier")
 * \return Aucun retour significatif
 */
int afficherUsers(char *cde) {
	printf("[%s] Liste des users [%d]\n", cde, users.nbUsers);	
	for (int i=0; i < users.nbUsers; i++)
			printf("\tUser [%d:%s], Etat [%c]\n",
				i,users.tab[i].name,users.tab[i].etat);
}


/**
 * \fn int trouverUser(name_t nom)
 * \brief Recherche un utilisateur dans la liste par son nom
 * \param nom Nom de l'utilisateur à rechercher
 * \return Index de l'utilisateur s'il existe, -1 sinon
 */
int trouverUser(name_t nom) {
	int i=0;
	for (i=0; i < MAX_USERS; i++)
		if (strcmp(nom, users.tab[i].name)==0) 
			return i;
	return -1;
}


/**
 * \fn int creerUser(name_t nom, char * adrIP, short port)
 * \brief Crée un nouvel utilisateur et l'ajoute à la liste
 * \param nom Nom de l'utilisateur
 * \param adrIP Adresse IP de l'utilisateur
 * \param port Port du serveur applicatif associé
 * \return Index du nouvel utilisateur, -1 si la liste est pleine
 */
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

/**
 * \fn int identifierUser(char * userName, char * adrIP, short port)
 * \brief Identifie un utilisateur : le crée s'il n'existe pas ou met à jour ses informations
 * \param userName Nom de l'utilisateur
 * \param adrIP Adresse IP de l'utilisateur
 * \param port Port du serveur applicatif
 * \return Index de l'utilisateur
 */
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

/**
 * \fn void deconnecterUser(int indUser)
 * \brief Déconnecte un utilisateur et libère ses ressources
 * \param indUser Index de l'utilisateur à déconnecter
 */
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

/**
 * \fn void modifierEtat(int indUser, etat_joueur_t etat)
 * \brief Modifie l'état d'un utilisateur
 * \param indUser Index de l'utilisateur
 * \param etat Nouvel état
 */
void modifierEtat(int indUser, etat_joueur_t etat) {
	users.tab[indUser].etat = etat;
	
	//
	afficherUsers("modifier");
	//return indDest;
}

/**
 * \fn char * nameUser(int indUser)
 * \brief Retourne le nom d'un utilisateur
 * \param indUser Index de l'utilisateur
 * \return Nom de l'utilisateur ou NULL si index invalide
 */
char * nameUser(int indUser) {
	if (indUser==-1) 
		return NULL;
	else 
		return users.tab[indUser].name;
}

/**
 * \fn void lireUsers(void)
 * \brief Charge la liste des utilisateurs depuis un fichier
 */
void lireUsers(void) {
	FILE *fp;
	CHECK_NULL(fp=fopen("users.dat", "r"), "--fopen()--");
	CHECK(users.nbUsers=fread(users.tab, sizeof(user_t), MAX_USERS, fp),"--fread()--");
	CHECK(fclose(fp),"--fclose()--");	

	afficherUsers("lecture");
}

/**
 * \fn void ecrireUsers(void)
 * \brief Sauvegarde la liste des utilisateurs dans un fichier
 */
void ecrireUsers(void) {
	FILE *fp;
	CHECK_NULL(fp=fopen("users.dat", "w"), "--fopen()--");
	CHECK(fwrite(users.tab, sizeof(user_t), users.nbUsers, fp),"--fread()--");
	CHECK(fclose(fp),"--fclose()--");	
	
	afficherUsers("ecriture");
}

/**
 * \fn void getListPseudoByState(etat_joueur_t etat, char * listePseudo)
 * \brief Récupère la liste des pseudos des utilisateurs selon leur état
 * \param etat État des utilisateurs recherchés
 * \param listePseudo Buffer de sortie contenant les pseudos séparés par ':'
 */
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

/**
 * \fn void getDetailsUser(int indUser, char * detailsUser)
 * \brief Récupère les informations détaillées d'un utilisateur
 * \param indUser Index de l'utilisateur
 * \param detailsUser Buffer de sortie formaté (etat:IP:port)
 */
void getDetailsUser(int indUser, char * detailsUser){
	snprintf(detailsUser, TAILLE_OPT, "%c:%s:%hu", users.tab[indUser].etat, users.tab[indUser].adrIP, users.tab[indUser].port_srv_app); 
}