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

socket_t se; 


int main(int argc, char ** argv) {
    socket_t sd;

    printf("Hello, MCS !\n");

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