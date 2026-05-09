/**
 * \file server.c
 * \brief Implémentation du serveur d'enregistrement
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#include <stdio.h>
#include <unistd.h> // POSIX
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "data.h"
#include "session.h"
#include "dial.h"
#include "reqRep.h"
#include "users.h"

/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */

/**
 * \def defaultAdrPort
 * \brief Adresse d'écoute par défaut du serveur (toutes les interfaces)
 */
#define defaultAdrPort "0.0.0.0"

/**
 * \def defaultPort
 * \brief Port d'écoute par défaut (0 = attribution automatique par l'OS)
 */
#define defaultPort 0


/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    G L O B A L E S
 */

/**
 * \var se
 * \brief Socket d'écoute principale du serveur d'enregistrement
 */
socket_t se; 


/**
 * \var MUT_USER_MANAGEMENT
 * \brief Mutex protégeant les accès concurrents à la liste des utilisateurs enregistrés
 */
pthread_mutex_t MUT_USER_MANAGEMENT = PTHREAD_MUTEX_INITIALIZER;


/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * \fn void installSigServer(int sigNum)
 * \brief Installe le gestionnaire de signal pour le serveur
 * \param sigNum Numéro du signal à intercepter
 */
void installSigServer(int sigNum);


/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

/**
 * \fn int main(int argc, char ** argv)
 * \brief Fonction principale du serveur d'enregistrement
 * \param argc Nombre d'arguments de ligne de commande
 * \param argv Tableau des arguments (argv[1] = adresse IP, argv[2] = port)
 * \return Code de retour du programme
 */
int main(int argc, char ** argv) {
    socket_t sd;


    installSigServer(SIGUSR1);

    printf("Hello, MCS !\n");

    lireUsers();

    if (argc < 2) {

        printf("Usage : %s adrIP port\n", argv[0]);
        // exit(EXIT_FAILURE);

        se = creerSocketEcoute(defaultAdrPort, defaultPort); 

    }
    else{

        se = creerSocketEcoute(argv[1], atoi(argv[2]));
    
    }
    
    while(1) {
        
        sd = accepterClt(se);

        pthread_t th;
        socket_t *sd_p = (socket_t*)malloc(sizeof(socket_t));
        *sd_p = sd;
        pthread_create(&th, NULL, (pFctThread)dialReg2Clt, (void*)sd_p);
        pthread_detach(th); 
    }
    close(se.fd); // la socket d'écoute n'est jamais fermée


    return 0;
}





/**
 * \fn void deroute (int sigNum)
 * \brief Gestionnaire de signal du serveur (SIGCHLD et SIGUSR1)
 * \param sigNum Numéro du signal reçu
 */
void deroute (int sigNum) {
    int status;
    switch (sigNum) {
        case SIGCHLD : 
            wait(&status); 
            break;
        case SIGUSR1 : 
            close(se.fd);
            ecrireUsers();
            printf("\x1b[1;31mFin du serveur [%d]\x1b[0m\n", getpid());
            _exit(1);  // _exit plutot que exit pour que ça marche dans un contexte multithread, car on ne touche pas aux buffers stdin/stderr, et qu'il ne prend pas de verrous
            break;
    }
}


/**
 * \fn void installSigServer(int sigNum)
 * \brief Installe un gestionnaire de signal via sigaction
 * \param sigNum Numéro du signal à intercepter
 */
void installSigServer(int sigNum) {
    struct sigaction newAct;
    newAct.sa_handler = deroute;
    newAct.sa_flags = SA_RESTART;
    sigemptyset(&newAct.sa_mask) ;
    CHECK(sigaction(sigNum, &newAct, NULL),"--sigaction()--");
}

