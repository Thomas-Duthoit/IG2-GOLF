#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <raylib/raylib.h>

#include "data.h"
#include "session.h"
#include "reqRep.h"
#include "dial.h"
#include "users.h"


#define printAppSrv(fmt, ...) printf("\x1b[1;36mAPP SERVER\x1b[0m] " fmt, ##__VA_ARGS__)



void * serv_applicatif(void * arg);
int get_local_ip(char *buffer);



requete_t *req_send_clt2reg = NULL;
pthread_mutex_t MUT_CLT2REG = PTHREAD_MUTEX_INITIALIZER;

short PORT_SRV_REG;
short PORT_SRV_APP = 0;
char INTERFACE[50]; 
char IP_REG[100];
char IP_LOC[100];



socket_t sa_reg;
socket_t se;



int main(int argc, char **argv) {
    
    // InitWindow(800, 450, "Hello, MCS !");

    // while (!WindowShouldClose())
    // {
    //     BeginDrawing();
    //         ClearBackground(RAYWHITE);
    //         DrawText("Hello, MCS !", 190, 200, 20, LIGHTGRAY);

    //         DrawFPS(10, 10);
    //     EndDrawing();
    // }

    // CloseWindow();


    if (argc < 4) {
        printf("Usage : %s adrIP port interface\n", argv[0]);
        exit(EXIT_FAILURE);
    } else {
        PORT_SRV_REG = atoi(argv[2]);
        strcpy(IP_REG, argv[1]);
        struct sockaddr_in addr = getIPAddr(argv[3]);
        strcpy(IP_LOC, inet_ntoa(addr.sin_addr));

    }


    name_t pseudo;
    printf("PSEUDO : ");
    fgets(pseudo, MAX_NAME, stdin);
    pseudo[strlen(pseudo)-1] = '\0';


    pthread_t th_clt2reg;
    pthread_t th_app_srv;

    requete_t req;

    pthread_create(&th_clt2reg, NULL, dialClt2Reg, (void*)&sa_reg);
    pthread_detach(th_clt2reg);
    pthread_create(&th_app_srv, NULL, serv_applicatif, NULL);
    pthread_detach(th_app_srv);


    while (PORT_SRV_APP == 0) { } // attente de l'attribution du port avant la connection au serveur d'neregistrement
    

    

    pthread_mutex_lock(&MUT_CLT2REG);

        req.idReq=REG_PLAYER;
        strcpy(req.verbReq, "REG");
        sprintf(req.optReq, "%s:%s:%hu", pseudo, IP_LOC, PORT_SRV_APP);

        if (req_send_clt2reg == NULL) {
            req_send_clt2reg = malloc(sizeof(requete_t));
            *req_send_clt2reg = req;
        }        


    pthread_mutex_unlock(&MUT_CLT2REG);

    while (req_send_clt2reg != NULL) {};  // attente de l'envoi confirmé

    char buff[MAX_BUFFER];
    char line[10];
    char c = 'X';


    while (1) {

        printf("nouvel état : ");
    
        if (fgets(line, sizeof(line), stdin)) {
            c = line[0];
        }

        if (c == '.') {

            break;
        }

        pthread_mutex_lock(&MUT_CLT2REG);

            req.idReq=UPDT_CLIENT_STATE;
            strcpy(req.verbReq, "UPDT");
            
            
            snprintf(req.optReq, TAILLE_OPT, "%s:%c", pseudo, c);
                        
            if (req_send_clt2reg == NULL) {
                req_send_clt2reg = malloc(sizeof(requete_t));
                *req_send_clt2reg = req;
            }


        pthread_mutex_unlock(&MUT_CLT2REG);


        req2str(&req, buff);

        while (req_send_clt2reg != NULL) {};  // attente de l'envoi confirmé


        pthread_mutex_lock(&MUT_CLT2REG);

            req.idReq=GET_HOSTS_LIST;
            strcpy(req.verbReq, "GET_H_LIST");
            strcpy(req.optReq, "");

            if (req_send_clt2reg == NULL) {
                req_send_clt2reg = malloc(sizeof(requete_t));
                *req_send_clt2reg = req;
            }

        pthread_mutex_unlock(&MUT_CLT2REG);

        req2str(&req, buff);

        while (req_send_clt2reg != NULL) {};  // attente de l'envoi confirmé

    }


    // Déconnexion
    pthread_mutex_lock(&MUT_CLT2REG);

        req.idReq=DIS_PLAYER;
        strcpy(req.verbReq, "DIS_PLAYER");
        strcpy(req.optReq, pseudo);
                    
        if (req_send_clt2reg == NULL) {
            req_send_clt2reg = malloc(sizeof(requete_t));
            *req_send_clt2reg = req;
        }

    pthread_mutex_unlock(&MUT_CLT2REG);


    req2str(&req, buff);
    while (req_send_clt2reg != NULL) {};  // attente de l'envoi confirmé

    pthread_mutex_lock(&MUT_CLT2REG);

        req.idReq=END_DIAL;
        strcpy(req.verbReq, "END");
        strcpy(req.optReq, "");
                    
        if (req_send_clt2reg == NULL) {
            req_send_clt2reg = malloc(sizeof(requete_t));
            *req_send_clt2reg = req;
        }

    pthread_mutex_unlock(&MUT_CLT2REG);


    req2str(&req, buff);
    while (req_send_clt2reg != NULL) {};  // attente de l'envoi confirmé

    return 0;
}





void * serv_applicatif(void * arg) {

    socklen_t lenMyAddr = sizeof(se.addrLoc);
    struct hostent *host; 
    printAppSrv("APP SERVER thread lance !\n");

    se = creerSocketEcoute("0.0.0.0", 0);

    CHECK(getsockname(se.fd, (struct sockaddr *)&se.addrLoc, &lenMyAddr),"--getsockname()--");    
    PORT_SRV_APP = se.addrLoc.sin_port;


    pthread_exit(EXIT_SUCCESS);
}



