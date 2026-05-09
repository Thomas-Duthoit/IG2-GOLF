/**
 * \file dial.h
 * \brief Déclarations pour les dialogues client-serveur
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#ifndef DIAL_H
#define DIAL_H

/**
 * \def NB_JOUEURS_MAX
 * \brief Nombre maximum de joueurs
 */
#define NB_JOUEURS_MAX 6


/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */

#ifdef DEBUG
    /**
     * \def printClt2Reg(fmt, ...)
     * \brief Macro de debug pour les communications client vers serveur d'enregistrement
     */
    #define printClt2Reg(fmt, ...) printf("\x1b[1;32mDIAL REG\x1b[0m] " fmt, ##__VA_ARGS__)
#else
    #define printClt2Reg(fmt, ...)
#endif

/**
 * \def printReg2Clt(fmt, ...)
 * \brief Macro pour les communications serveur d'enregistrement vers client
 */
#define printReg2Clt(fmt, ...) printf("\x1b[1;35mREGISTER SERVER\x1b[0m] " fmt, ##__VA_ARGS__)

/**
 * \def printApp2Clt(fmt, ...)
 * \brief Macro pour les communications serveur applicatif vers client
 */
#define printApp2Clt(fmt, ...) printf("\x1b[1;36mAPP SERVER (app2clt)\x1b[0m] " fmt, ##__VA_ARGS__)

/**
 * \def printAppSrv(fmt, ...)
 * \brief Macro pour le serveur applicatif
 */
#define printAppSrv(fmt, ...) printf("\x1b[1;36mAPP SERVER\x1b[0m] " fmt, ##__VA_ARGS__)

/**
 * \def printClt2App(fmt, ...)
 * \brief Macro pour les communications client vers serveur applicatif
 */
#define printClt2App(fmt, ...) printf("\x1b[1;36mAPP SERVER (clt2app)\x1b[0m] " fmt, ##__VA_ARGS__)

/**
 * \def printMulticast(fmt, ...)
 * \brief Macro pour les communications multicast
 */
#define printMulticast(fmt, ...) printf("\x1b[1;37;41mMULTICAST\x1b[0m] " fmt, ##__VA_ARGS__)

/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */

/**
 * \typedef pFctThread
 * \brief Pointeur de fonction pour les threads
 */
typedef void * (*pFctThread)(void *);


/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

// ---- SERVEUR D'ENREGISTREMENT | CLIENT -----------
/**
 * \fn void * dialReg2Clt(socket_t * sd)
 * \brief Thread pour le dialogue serveur d'enregistrement vers client
 * \param sd Socket du client
 * \return NULL
 */
void * dialReg2Clt(socket_t * sd);

/**
 * \fn void * dialClt2Reg(socket_t * sa)
 * \brief Thread pour le dialogue client vers serveur d'enregistrement
 * \param sa Socket d'appel
 * \return NULL
 */
void * dialClt2Reg(socket_t * sa);

// ---- SERVEUR D'APPLICATION | CLIENT -----------
/**
 * \fn void * dialApp2Clt(socket_t * sd)
 * \brief Thread pour le dialogue serveur applicatif vers client
 * \param sd Socket du client
 * \return NULL
 */
void * dialApp2Clt(socket_t * sd);

/**
 * \fn void * dialClt2App(socket_t * sa)
 * \brief Thread pour le dialogue client vers serveur applicatif
 * \param sa Socket d'appel
 * \return NULL
 */
void * dialClt2App(socket_t * sa);

/**
 * \fn void * multiSendToClts(socket_t * sm)
 * \brief Thread pour l'envoi multicast aux clients
 * \param sm Socket multicast
 * \return NULL
 */
void * multiSendToClts(socket_t * sm);  // serv app -> multicast aux clients

/**
 * \fn void * multiRecvFromApp(socket_t * sam)
 * \brief Thread pour la réception multicast depuis l'application
 * \param sam Socket d'appel multicast
 * \return NULL
 */
void * multiRecvFromApp(socket_t * sam);  // réception des messages envoyés en multicast





#endif  // DIAL_H
