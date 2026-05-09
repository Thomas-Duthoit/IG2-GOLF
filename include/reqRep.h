/**
 * \file reqRep.h
 * \brief Déclaration pour la conversion entre requêtes/réponses et chaînes de caractères pour la communication réseau
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */
#ifndef REQREP_H
#define REQREP_H


/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */

 
/**
 * \def TAILLE_VERB
 * \brief Taille maximale du verb
 */
#define TAILLE_VERB 20

/**
 * \def TAILLE_OPT
 * \brief Taille maximale des options
 */
#define TAILLE_OPT 500

/**
 * \def REQ_IN_OUT
 * \brief Format pour convertir requête en chaîne
 */
#define REQ_IN_OUT "%hu:%s:%s"          // req -> str

/**
 * \def REQ_OUT_IN
 * \brief Format pour convertir chaîne en requête
 */
#define REQ_OUT_IN "%hu:%[^:]:%[^\n]"    // str -> req

/**
 * \def REP_IN_OUT
 * \brief Format pour convertir réponse en chaîne
 */
#define REP_IN_OUT "%hu:%s:%s"          // rep -> str

/**
 * \def REP_OUT_IN
 * \brief Format pour convertir chaîne en réponse
 */
#define REP_OUT_IN "%hu:%[^:]:%[^\n]"    // str -> rep

/**
 * \struct requete_t
 * \brief Structure représentant une requête
 */
typedef struct {
    short idReq; /**< ID de la requête */
    char verbReq[TAILLE_VERB]; /**< Verbe de la requête */
    char optReq[TAILLE_OPT]; /**< Options de la requête */
} requete_t;

/**
 * \struct reponse_t
 * \brief Structure représentant une réponse
 */
typedef struct {
    short idRep; /**< ID de la réponse */
    char verbRep[TAILLE_VERB]; /**< Verbe de la réponse */
    char optRep[TAILLE_OPT]; /**< Options de la réponse */
} reponse_t;

/**
 * \enum listeReqRep_t
 * \brief Énumération des types de requêtes/réponses
 */
typedef enum {

    END_DIAL = 0, /**< Fin du dialogue */

    // Requêtes Clients -> Serveur d’enregistrement
    REG_PLAYER = 001, /**< Enregistrement d'un joueur */
    DIS_PLAYER, /**< Déconnexion d'un joueur */
    GET_HOSTS_LIST, /**< Obtenir la liste des hôtes */
    UPDT_CLIENT_STATE, /**< Mise à jour de l'état client */
    GET_PLAYER_FROM_ID, /**< Obtenir un joueur par ID */

    // Requêtes Clients -> Serveur applicatif
    JOIN_GAME = 101, /**< Rejoindre une partie */
    GET_PLAYERS_LIST, /**< Obtenir la liste des joueurs */
    LEAVE_GAME, /**< Quitter une partie */
    SHOOT, /**< Tirer */

    // Requêtes Serveur applicatif -> Clients
    START_GAME = 201, /**< Démarrer la partie */
    SET_BALL_VEL, /**< Définir la vélocité de la balle */
    SET_BALL_POS, /**< Définir la position de la balle */
    NEXT_PLAYER_TO_PLAY, /**< Prochain joueur à jouer */
    START_NEXT_ROUND, /**< Démarrer la prochaine manche */
    END_GAME, /**< Fin de la partie */
    END_SERV, /**< Fin du serveur */

    // Réponses Serveur d’enregistrement -> Clients
    OK_REG_SERV = 301, /**< OK enregistrement serveur */
    HOST_LIST, /**< Liste des hôtes */
    PLAYER_DETAILS, /**< Détails du joueur */
    ERR_REG_SERV, /**< Erreur enregistrement serveur */

    // Réponses Serveur applicatif -> Clients
    OK_APP_SERV = 401, /**< OK serveur applicatif */
    NOK_APP_SERV, /**< NOK serveur applicatif */
    PLAYERS_LIST /**< Liste des joueurs */

} listeReqRep_t;



/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

/**
 * \fn void req2str(requete_t * req, char * str) 
 * \brief Sérialise une requête en chaîne de caractères pour l'envoi réseau
 * \param req Pointeur vers la requête source
 * \param str Chaîne de destination (doit être suffisamment grande)
 */
void req2str(requete_t * req, char * str);

/**
 * \fn void str2req(char * str, requete_t * req)
 * \brief Désérialise une chaîne de caractères reçue du réseau en structure requête
 * \param str Chaîne source
 * \param req Pointeur vers la requête destination
 */
void str2req(char * str, requete_t * req);

/**
 * \fn void rep2str(requete_t * rep, char * str)
 * \brief Sérialise une réponse en chaîne de caractères pour l'envoi réseau
 * \param rep Pointeur vers la réponse source
 * \param str Chaîne de destination (doit être suffisamment grande)
 */
void rep2str(requete_t * rep, char * str);

/**
 * \fn void str2rep(char * str, requete_t * rep)
 * \brief Désérialise une chaîne de caractères reçue du réseau en structure réponse
 * \param str Chaîne source
 * \param rep Pointeur vers la réponse destination
 */
void str2rep(char * str, requete_t * rep);

#endif /* REQREP_H */