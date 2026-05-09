/**
 * \file users.h
 * \brief Déclaration pour la gestion des utilisateurs (création, identification, état, persistance)
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#ifndef USERS_H
#define USERS_H

/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "session.h"


/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */

/**
 * \def MAX_NAME
 * \brief Taille maximale du nom
 */
#define MAX_NAME 64

/**
 * \def MAX_USERS
 * \brief Nombre maximum d'utilisateurs
 */
#define MAX_USERS	16

/**
 * \typedef name_t
 * \brief Type pour les noms
 */
typedef char name_t[MAX_NAME];

/**
 * \struct user_t
 * \brief Structure représentant un utilisateur
 */
typedef struct {
	name_t name; /**< Nom de l'utilisateur */
	char * adrIP; /**< Adresse IP */
	short port_srv_app; /**< Port du serveur applicatif */
	char etat; /**< État de l'utilisateur */
} user_t;

/**
 * \struct users_t
 * \brief Structure pour la liste des utilisateurs
 */
typedef struct {
	user_t tab[MAX_USERS]; /**< Tableau des utilisateurs */
	int nbUsers; /**< Nombre d'utilisateurs */
} users_t;

/**
 * \enum etat_joueur_t
 * \brief Énumération des états des joueurs
 */
typedef enum {
	ETAT_OFFLINE='X', /**< Hors ligne */
	ETAT_ONLINE='O', /**< En ligne */
	ETAT_HOST='H', /**< Hôte */
	ETAT_FULL='F' /**< Complet */
} etat_joueur_t;

/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

/**
 * \fn int trouverUser(name_t nom)
 * \brief Recherche un utilisateur dans la liste par son nom
 * \param nom Nom de l'utilisateur à rechercher
 * \return Index de l'utilisateur s'il existe, -1 sinon
 */
int trouverUser(char *nom);

/**
 * \fn int creerUser(name_t nom, char * adrIP, short port)
 * \brief Crée un nouvel utilisateur et l'ajoute à la liste
 * \param nom Nom de l'utilisateur
 * \param adrIP Adresse IP de l'utilisateur
 * \param port Port du serveur applicatif associé
 * \return Index du nouvel utilisateur, -1 si la liste est pleine
 */
int creerUser(name_t nom, char * adrIP, short port);

/**
 * \fn int identifierUser(char * userName, char * adrIP, short port)
 * \brief Identifie un utilisateur : le crée s'il n'existe pas ou met à jour ses informations
 * \param userName Nom de l'utilisateur
 * \param adrIP Adresse IP de l'utilisateur
 * \param port Port du serveur applicatif
 * \return Index de l'utilisateur
 */
int identifierUser(char * userName, char * adrIP, short port);

/**
 * \fn void deconnecterUser(int indUser)
 * \brief Déconnecte un utilisateur et libère ses ressources
 * \param indUser Index de l'utilisateur à déconnecter
 */
void deconnecterUser(int indUser);

/**
 * \fn void modifierEtat(int indUser, etat_joueur_t etat)
 * \brief Modifie l'état d'un utilisateur
 * \param indUser Index de l'utilisateur
 * \param etat Nouvel état
 */
void modifierEtat(int indUser, etat_joueur_t etat);

/**
 * \fn char * nameUser(int indUser)
 * \brief Retourne le nom d'un utilisateur
 * \param indUser Index de l'utilisateur
 * \return Nom de l'utilisateur ou NULL si index invalide
 */
char * nameUser(int indUser);

/**
 * \fn void lireUsers(void)
 * \brief Charge la liste des utilisateurs depuis un fichier
 */
void lireUsers(void);

/**
 * \fn void ecrireUsers(void)
 * \brief Sauvegarde la liste des utilisateurs dans un fichier
 */
void ecrireUsers(void);

/**
 * \fn void getListPseudoByState(etat_joueur_t etat, char * listePseudo)
 * \brief Récupère la liste des pseudos des utilisateurs selon leur état
 * \param etat État des utilisateurs recherchés
 * \param listePseudo Buffer de sortie contenant les pseudos séparés par ':'
 */
void getListPseudoByState(etat_joueur_t etat, char * listePseudo);

/**
 * \fn void getDetailsUser(int indUser, char * detailsUser)
 * \brief Récupère les informations détaillées d'un utilisateur
 * \param indUser Index de l'utilisateur
 * \param detailsUser Buffer de sortie formaté (etat:IP:port)
 */
void getDetailsUser(int indUser, char * detailsUser);

#endif /* USERS_H */ 
