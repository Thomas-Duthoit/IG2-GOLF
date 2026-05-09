/**
 * \file update.h
 * \brief Déclaration pour la mise à jour de la logique de jeu selon les états
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#ifndef UPDATE_H
#define UPDATE_H

/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */

/**
 * \def printIHM(fmt, ...)
 * \brief Macro pour les messages IHM
 */
#define printIHM(fmt, ...) printf("\x1b[1;31mIHM (MAIN)\x1b[0m] " fmt, ##__VA_ARGS__)

/**
 * \def envoi_avec_ack(cond_debut, cond_fin, mut_fin)
 * \brief Macro pour envoyer avec acquittement
 */
#define envoi_avec_ack(cond_debut, cond_fin, mut_fin) pthread_cond_signal(&(cond_debut)); pthread_cond_wait(&(cond_fin), &(mut_fin));

/**
 * \def envoi_no_ack(cond_debut)
 * \brief Macro pour envoyer sans acquittement
 */
#define envoi_no_ack(cond_debut) pthread_cond_signal(&(cond_debut));  // pas d'attente d'ACK, par exmeple pour le END_DIAL

/**
 * @def estHote()
 * @brief Macro pour vérifier si le joueur est l'hôte
 */
#define estHote() (strcmp(hote_serv_app.name, pseudo) == 0) && (strcmp(hote_serv_app.adrIP, IP_SERVICE) == 0) && (hote_serv_app.port_srv_app == PORT_SRV_APP)


/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * \def NB_MANCHE
 * \brief Nombre de manches
 */
#define NB_MANCHE 3

/**
 * \enum game_state_t
 * \brief Énumération des états du jeu
 */
typedef enum {
    LIST = 1, /**< État liste */
    LOBBY_HOTE, /**< Lobby hôte */
    LOBBY_CLIENT, /**< Lobby client */
    START, /**< Départ */
    GAME, /**< Jeu */
    END, /**< Fin */
    NEXT, /**< Prochain */
    PODIUM, /**< Podium */
} game_state_t;

/**
 * \enum cam_mode_t
 * \brief Énumération des modes de caméra
 */
typedef enum {
    CAM_MODE_FREE, /**< Caméra libre */
    CAM_MODE_BALL, /**< Caméra sur la balle */
} cam_mode_t;

/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

/**
 * \fn void updateLIST()
 * \brief Met à jour la logique de l'état LIST (liste des hôtes disponibles)
 */
void updateLIST();

/**
 * \fn void updateLOBBY()
 * \brief Met à jour la logique de l'état LOBBY pour l'hôte (gestion des joueurs et démarrage de partie)
 */
void updateLOBBY();

/**
 * \fn void updateLOBBYClt()
 * \brief Met à jour la logique de l'état LOBBY pour le client (gestion déconnexion et quitter)
 */
void updateLOBBYClt();

/**
 * \fn void updateGAME()
 * \brief Met à jour la logique de l'état GAME (physique des balles, tir, passage au joueur suivant)
 */
void updateGAME();

/**
 * \fn void updateEND()
 * \brief Met à jour la logique de l'état END (retour au lobby après la fin de partie)
 */
void updateEND();

#endif  // UPDATE_H