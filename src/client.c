#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <raylib/raylib.h>

#include "data.h"
#include "session.h"
#include "reqRep.h"
#include "dial.h"
#include "users.h"




// requête pour envoie sur 
requete_t *req_send_clt2reg = NULL;

// mutex pour la comm avec le serveur
pthread_mutex_t MUT_CLT2REG = PTHREAD_MUTEX_INITIALIZER;



int main(int argc, char **argv) {
    printf("Hello, MCS !\n");
    
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


    if (argc < 2) {
        printf("Usage : %s adrIP port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    name_t pseudo;
    printf("PSEUDO : ");
    fgets(pseudo, MAX_NAME, stdin);
    pseudo[strlen(pseudo)-1] = '\0';


    pthread_t th_clt2reg;
    requete_t req;
    socket_t sa_reg = connecterClt2Srv(argv[1], atoi(argv[2]));

    pthread_create(&th_clt2reg, NULL, dialClt2Reg, (void*)&sa_reg);
    pthread_detach(th_clt2reg);


    pthread_mutex_lock(&MUT_CLT2REG);

        req.idReq=REG_PLAYER;
        strcpy(req.verbReq, "TEST");
        strcpy(req.optReq, pseudo);

        if (req_send_clt2reg == NULL) {
            req_send_clt2reg = malloc(sizeof(requete_t));
            *req_send_clt2reg = req;
        }        


    pthread_mutex_unlock(&MUT_CLT2REG);

    printf("envoye !\n");

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
        printf("envoye : '%s' !\n", buff);

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
        printf("envoye : '%s' !\n", buff);

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
    printf("envoye : '%s' !\n", buff);
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
    printf("envoye : '%s' !\n", buff);
    while (req_send_clt2reg != NULL) {};  // attente de l'envoi confirmé

    return 0;
}