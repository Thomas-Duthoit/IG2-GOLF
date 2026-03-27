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



void installSigServer(int sigNum);




socket_t se; 


// mutex pour la gestion des utilisateurs
pthread_mutex_t MUT_USER_MANAGEMENT = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char ** argv) {
    socket_t sd;


    installSigServer(SIGUSR1);

    printf("Hello, MCS !\n");

    lireUsers();

    if (argc < 2) {
        printf("Usage : %s adrIP port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    se = creerSocketEcoute(argv[1], atoi(argv[2]));
    
    while(1) {
        
        sd = accepterClt(se);

        pthread_t th;
        socket_t *sd_p = (socket_t*)malloc(sizeof(socket_t));
        *sd_p = sd;
        pthread_create(&th, NULL, dialReg2Clt, (void*)sd_p);
        pthread_detach(th);        
    }
    close(se.fd); // la socket d'écoute n'est jamais fermée


    return 0;
}





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


void installSigServer(int sigNum) {
    struct sigaction newAct;
    newAct.sa_handler = deroute;
    newAct.sa_flags = SA_RESTART;
    sigemptyset(&newAct.sa_mask) ;
    CHECK(sigaction(sigNum, &newAct, NULL),"--sigaction()--");
}

