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

char buff_pseudos_hotes[TAILLE_OPT];


socket_t sa_reg;
socket_t se;



int main(int argc, char **argv) {

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

    

    // création de l'IHM avec raylib
    InitWindow(800, 450, "IG2-GOLF");  // fenêtre 800 x 450 px

    float delai_refresh_hosts=1.0;  // 1 s
    float tempo_refresh_hosts=0.0;

    while (!WindowShouldClose()) {  // tant que la fenêtre ne doit pas se fermer


        // partie IHM (clics, etc...)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

            // récupération de la position de la souris
            int mouse_x = GetMouseX();
            int mouse_y = GetMouseY();

            // bouton host
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 30, 30, 30})) {
                pthread_mutex_lock(&MUT_CLT2REG);

                req.idReq=UPDT_CLIENT_STATE;
                strcpy(req.verbReq, "UPDT");
                
                snprintf(req.optReq, TAILLE_OPT, "%s:H", pseudo);
                            
                if (req_send_clt2reg == NULL) {
                    req_send_clt2reg = malloc(sizeof(requete_t));
                    *req_send_clt2reg = req;
                }

                pthread_mutex_unlock(&MUT_CLT2REG);
            }

            // bouton online
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){50, 30, 30, 30})) {
                pthread_mutex_lock(&MUT_CLT2REG);

                req.idReq=UPDT_CLIENT_STATE;
                strcpy(req.verbReq, "UPDT");
                
                snprintf(req.optReq, TAILLE_OPT, "%s:O", pseudo);
                            
                if (req_send_clt2reg == NULL) {
                    req_send_clt2reg = malloc(sizeof(requete_t));
                    *req_send_clt2reg = req;
                }

                pthread_mutex_unlock(&MUT_CLT2REG);

            }

            // bouton full
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){90, 30, 30, 30})) {
                pthread_mutex_lock(&MUT_CLT2REG);

                req.idReq=UPDT_CLIENT_STATE;
                strcpy(req.verbReq, "UPDT");
                
                snprintf(req.optReq, TAILLE_OPT, "%s:F", pseudo);
                            
                if (req_send_clt2reg == NULL) {
                    req_send_clt2reg = malloc(sizeof(requete_t));
                    *req_send_clt2reg = req;
                }

                pthread_mutex_unlock(&MUT_CLT2REG);
            }
        }

        // partie envoie récurrents
        tempo_refresh_hosts += GetFrameTime();
        if (tempo_refresh_hosts >= delai_refresh_hosts) {
            
            tempo_refresh_hosts -= delai_refresh_hosts;

            pthread_mutex_lock(&MUT_CLT2REG);

            req.idReq=GET_HOSTS_LIST;
            strcpy(req.verbReq, "HOSTS");
            strcpy(req.optReq, "");
                        
            if (req_send_clt2reg == NULL) {
                req_send_clt2reg = malloc(sizeof(requete_t));
                *req_send_clt2reg = req;
            }

            pthread_mutex_unlock(&MUT_CLT2REG);
        }


        // partie affichage
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawText("ETAT :", 10, 10, 20, BLACK);

            // bouton host
            DrawRectangle(10, 30, 30, 30, GRAY);
            DrawText("H", 13, 33, 20, BLACK);

            // bouton online
            DrawRectangle(50, 30, 30, 30, GRAY);
            DrawText("O", 53, 33, 20, BLACK);

            // bouton full
            DrawRectangle(90, 30, 30, 30, GRAY);
            DrawText("F", 93, 33, 20, BLACK);

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {  // clic gauche

                // récupération de la position de la souris
                int mouse_x = GetMouseX();
                int mouse_y = GetMouseY();

                // bouton host
                if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 30, 30, 30})) {
                    DrawRectangle(10, 30, 30, 30, GREEN);
                    DrawText("H", 13, 33, 20, DARKGREEN);
                }

                // bouton online
                if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){50, 30, 30, 30})) {
                    DrawRectangle(50, 30, 30, 30, GREEN);
                    DrawText("O", 53, 33, 20, DARKGREEN);
                }

                // bouton full
                if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){90, 30, 30, 30})) {
                    DrawRectangle(90, 30, 30, 30, GREEN);
                    DrawText("F", 93, 33, 20, DARKGREEN);
                }
            }

            // affichage @IP + port
            DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_LOC, PORT_SRV_APP), 10, 70, 20, BLACK);
            DrawText("Liste des hotes : ", 10, 90, 20, BLACK);

            
            pthread_mutex_lock(&MUT_CLT2REG);

            //printf("%s\n", buff_pseudos_hotes);
            char copie_buff_pseudo_hotes[TAILLE_OPT];
            strcpy(copie_buff_pseudo_hotes, buff_pseudos_hotes);
            char * tok = strtok(copie_buff_pseudo_hotes, ":" );
            int i=0;
            while ( tok != NULL ) {
                DrawText(TextFormat("> %s", tok), 10, 110+(20*i), 20, BLACK);
                i++;
                tok = strtok(NULL, ":" );

            }

            pthread_mutex_unlock(&MUT_CLT2REG);


            DrawFPS(10, 450-20);
        
        EndDrawing();
    }

    CloseWindow();

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



