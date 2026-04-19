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


#define printIHM(fmt, ...) printf("\x1b[1;31mIHM (MAIN)\x1b[0m] " fmt, ##__VA_ARGS__)


typedef enum {
    LIST = 1,
    LOBBY_HOTE, 
    LOBBY_CLIENT, 

} game_state_t; 


void * serv_applicatif(void * arg);
void * requetes_recurrentes_reg_1s(void * arg);
int get_local_ip(char *buffer);
void updateLIST(); 
void renderLIST(); 
void updateLOBBY(); 
void renderLOBBY(); 
void updateLOBBYClt(); 
void renderLOBBYClt(); 


requete_t *req_send_clt2reg = NULL;
pthread_mutex_t MUT_CLT2REG = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t end_reqrep_clt2reg = PTHREAD_COND_INITIALIZER;
pthread_mutex_t MUT_END_REQREP_CLT2REG = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t MUT_CLT2APP;
requete_t *req_send_clt2app;
pthread_cond_t end_reqrep_clt2app;


short PORT_SRV_REG;
short PORT_SRV_APP = 0;
char INTERFACE[50]; 
char IP_REG[100];
char IP_SERVICE[100];

char buff_pseudos_hotes[TAILLE_OPT];
char buff_info_joueur[TAILLE_OPT];
users_t hotes;
users_t clients_app; 

user_t hote_serv_app;  

socket_t sa_reg;
socket_t se;
socket_t sa;

game_state_t game_state; 
name_t pseudo;

int main(int argc, char **argv) {

    if (argc < 4) {
        printf("Usage : %s adrIP port interface\n", argv[0]);
        exit(EXIT_FAILURE);
    } else {
        PORT_SRV_REG = atoi(argv[2]);
        strcpy(IP_REG, argv[1]);
        struct sockaddr_in addr = getIPAddr(argv[3]);
        strcpy(IP_SERVICE, inet_ntoa(addr.sin_addr));

    }

    for (int i=0; i<MAX_USERS; i++) {
        hotes.tab[i].adrIP = (char *)malloc(sizeof(char)*16); 
    }


    // Allocation pour hôte du serveur applicatif rejoint
    hote_serv_app.adrIP = (char *)malloc(sizeof(char)*16); 

    
    printf("PSEUDO : ");
    fgets(pseudo, MAX_NAME, stdin);
    pseudo[strlen(pseudo)-1] = '\0';

    game_state = LIST; 


    pthread_t th_clt2reg;
    pthread_t th_app_srv;
    pthread_t th_req_rec;

    requete_t req;

    sa_reg = connecterClt2Srv(IP_REG, PORT_SRV_REG);

    pthread_create(&th_clt2reg, NULL, (pFctThread)dialClt2Reg, (void*)&sa_reg);
    pthread_detach(th_clt2reg);
    pthread_create(&th_app_srv, NULL, serv_applicatif, NULL);
    pthread_detach(th_app_srv);


    while (PORT_SRV_APP == 0) { } // attente de l'attribution du port avant la connection au serveur d'neregistrement
    
    pthread_mutex_lock(&MUT_CLT2REG);

        req.idReq=REG_PLAYER;
        strcpy(req.verbReq, "REG_PLAYER");
        sprintf(req.optReq, "%s:%s:%hu", pseudo, IP_SERVICE, PORT_SRV_APP);

        if (req_send_clt2reg == NULL) {
            req_send_clt2reg = malloc(sizeof(requete_t));
            *req_send_clt2reg = req;
        }        

    pthread_mutex_unlock(&MUT_CLT2REG);

    while (req_send_clt2reg != NULL) {};  // attente de l'envoi confirmé

    

    // création de l'IHM avec raylib
    SetTraceLogLevel(LOG_WARNING);  // on ne veut pas les messages d'info
    InitWindow(800, 450, TextFormat("IG2-GOLF - %s", pseudo));  // fenêtre 800 x 450 px

    pthread_create(&th_req_rec, NULL, requetes_recurrentes_reg_1s, NULL);
    pthread_detach(th_req_rec);

    while (!WindowShouldClose()) {  // tant que la fenêtre ne doit pas se fermer

        if (game_state == LIST){
            updateLIST(); 
            renderLIST(); 
        }
        else if (game_state == LOBBY_HOTE){
            updateLOBBY();
            renderLOBBY();
        }
        else if (game_state == LOBBY_CLIENT){
            updateLOBBYClt();
            renderLOBBYClt();
        }

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
        strcpy(req.verbReq, "END_DIAL");
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
    printAppSrv("APP SERVER thread lance !\n");

    se = creerSocketEcoute("0.0.0.0", 0);

    CHECK(getsockname(se.fd, (struct sockaddr *)&se.addrLoc, &lenMyAddr),"--getsockname()--");    
    PORT_SRV_APP = ntohs(se.addrLoc.sin_port);

    socket_t sd;

     while(1) {
        
        printAppSrv("Attente de connexion\n");
        sd = accepterClt(se);

        pthread_t th;
        socket_t *sd_p = (socket_t*)malloc(sizeof(socket_t));
        *sd_p = sd;
        pthread_create(&th, NULL, (pFctThread)dialApp2Clt, (void*)sd_p);
        pthread_detach(th);        
    }

    close(se.fd); // la socket d'écoute n'est jamais fermée


    pthread_exit(EXIT_SUCCESS);
}





void * requetes_recurrentes_reg_1s(void * arg) {

    
    // partie envoie récurrents
    // contenu de la boucle exécuté une fois par seconde

    requete_t req;

    while (1) {
        sleep(1);        

        pthread_mutex_lock(&MUT_CLT2REG);

        req.idReq=GET_HOSTS_LIST;
        strcpy(req.verbReq, "GET_HOSTS_LIST");
        strcpy(req.optReq, "");
                    
        if (req_send_clt2reg == NULL) {
            req_send_clt2reg = malloc(sizeof(requete_t));
            *req_send_clt2reg = req;
        }

        pthread_mutex_unlock(&MUT_CLT2REG);

        pthread_cond_wait(&end_reqrep_clt2reg, &MUT_END_REQREP_CLT2REG);  // attente fin de comm


        pthread_mutex_lock(&MUT_CLT2REG);

        char copie_buff_pseudo_hotes[TAILLE_OPT];
        strcpy(copie_buff_pseudo_hotes, buff_pseudos_hotes);
        char * tok = strtok(copie_buff_pseudo_hotes, ":" );
        int i=0;
        while ( tok != NULL ) {
            strcpy(hotes.tab[i].name, tok);
            i++;
            tok = strtok(NULL, ":" );
        }
        hotes.nbUsers = i;
        pthread_mutex_unlock(&MUT_CLT2REG);

        for (int j=0; j<hotes.nbUsers; j++) {
            pthread_mutex_lock(&MUT_CLT2REG);

            req.idReq=GET_PLAYER_FROM_ID;
            strcpy(req.verbReq, "GET_PLAYER_FROM_ID");
            strcpy(req.optReq, hotes.tab[j].name);
                        
            if (req_send_clt2reg == NULL) {
                req_send_clt2reg = malloc(sizeof(requete_t));
                *req_send_clt2reg = req;
            }

            pthread_mutex_unlock(&MUT_CLT2REG);

            pthread_cond_wait(&end_reqrep_clt2reg, &MUT_END_REQREP_CLT2REG);  // attente fin de comm


            sscanf(buff_info_joueur, "%c:%[^:]:%hu\n", &hotes.tab[j].etat, hotes.tab[j].adrIP, &hotes.tab[j].port_srv_app);

        }

    }

}


void updateLIST(){

    requete_t req;

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // bouton host
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 30, 30, 30})) {
            pthread_mutex_lock(&MUT_CLT2REG);

            req.idReq=UPDT_CLIENT_STATE;
            strcpy(req.verbReq, "UPDT_CLIENT_STATE");
            
            snprintf(req.optReq, TAILLE_OPT, "%s:H", pseudo);
                        
            if (req_send_clt2reg == NULL) {
                req_send_clt2reg = malloc(sizeof(requete_t));
                *req_send_clt2reg = req;
            }

            pthread_mutex_unlock(&MUT_CLT2REG);

            game_state = LOBBY_HOTE; 
        }

        // bouton online
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){50, 30, 30, 30})) {
            pthread_mutex_lock(&MUT_CLT2REG);

            req.idReq=UPDT_CLIENT_STATE;
            strcpy(req.verbReq, "UPDT_CLIENT_STATE");
            
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
            strcpy(req.verbReq, "UPDT_CLIENT_STATE");
            
            snprintf(req.optReq, TAILLE_OPT, "%s:F", pseudo);
                        
            if (req_send_clt2reg == NULL) {
                req_send_clt2reg = malloc(sizeof(requete_t));
                *req_send_clt2reg = req;
            }

            pthread_mutex_unlock(&MUT_CLT2REG);
        }

        for (int i=0; i<hotes.nbUsers; i++) {
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 110+(20*i), 20, 20})) {
                printIHM("Creation du thread de comm \"dialClt2App\" ...\n");

                pthread_t th_dialClt2App;

                sa = connecterClt2Srv(hotes.tab[i].adrIP, hotes.tab[i].port_srv_app);

                pthread_create(&th_dialClt2App, NULL, (pFctThread)dialClt2App, (void*)&sa);
                pthread_detach(th_dialClt2App);



                pthread_mutex_lock(&MUT_CLT2APP);

                req.idReq=JOIN_GAME;
                strcpy(req.verbReq, "JOIN_GAME");
                strcpy(req.optReq, pseudo);
                                            
                if (req_send_clt2app == NULL) {
                    req_send_clt2app = malloc(sizeof(requete_t));
                    *req_send_clt2app = req;
                }

                pthread_mutex_unlock(&MUT_CLT2APP);

                printIHM("... Connexion\n");
                
                strcpy(hote_serv_app.name, pseudo); 

                game_state = LOBBY_CLIENT; 
            }
        }
    }

}


void renderLIST(){
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

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

            
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {  // clic gauche


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
        DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 10, 70, 20, BLACK);
        DrawText("Liste des hotes : ", 10, 90, 20, BLACK);

        for (int i=0; i<hotes.nbUsers; i++) {
            DrawRectangle(10, 110+(20*i), 20, 20, GRAY);
            DrawText(">", 10, 110+(20*i), 20, BLACK);
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 110+(20*i), 20, 20})) {
                DrawRectangle(10, 110+(20*i), 20, 20, GREEN);
                DrawText(">", 10, 110+(20*i), 20, DARKGREEN);
            }
            DrawText(TextFormat("> %s  - %s : %hu", hotes.tab[i].name, hotes.tab[i].adrIP, hotes.tab[i].port_srv_app), 40, 110+(20*i), 20, BLACK);
        }



        DrawFPS(10, 450-20);
    
    EndDrawing();
}


void updateLOBBY(){
    requete_t req; 

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        
        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            
            pthread_mutex_lock(&MUT_CLT2REG);

            req.idReq=UPDT_CLIENT_STATE;
            strcpy(req.verbReq, "UPDT_CLIENT_STATE");
            
            snprintf(req.optReq, TAILLE_OPT, "%s:O", pseudo);
                        
            if (req_send_clt2reg == NULL) {
                req_send_clt2reg = malloc(sizeof(requete_t));
                *req_send_clt2reg = req;
            }

            pthread_mutex_unlock(&MUT_CLT2REG);

            game_state = LIST; 

        }

    }

} 




void renderLOBBY(){
    // partie affichage
    BeginDrawing();

        ClearBackground(RAYWHITE);

        // bouton quitter
        DrawRectangle(760, 30, 30, 30, GRAY);
        DrawText("X", 770, 35, 20, BLACK);


        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            DrawRectangle(760, 30, 30, 30, RED);
            DrawText("X", 770, 35, 20, DARKGRAY);
        }


        // affichage @IP + port
        DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 10, 10, 20, BLACK);
        
        
        for (int i=0; i<clients_app.nbUsers; i++) {
            DrawText(TextFormat("> %s ", clients_app.tab[i].name), 40, 110+(20*i), 20, BLACK);
        }


        DrawFPS(10, 450-20);

    EndDrawing();
}




void updateLOBBYClt(){
    requete_t req; 

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        
        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            // Déconnexion du serveur applicatif
            printIHM("Déconnexion de la partie ...\n"); 

            pthread_mutex_lock(&MUT_CLT2APP);

            req.idReq=LEAVE_GAME;
            strcpy(req.verbReq, "LEAVE_GAME");
            
            snprintf(req.optReq, TAILLE_OPT, "%s", pseudo);
                        
            if (req_send_clt2app == NULL) {
                req_send_clt2app = malloc(sizeof(requete_t));
                *req_send_clt2app = req;
            }

            pthread_mutex_unlock(&MUT_CLT2APP);


            game_state = LIST; 

            printIHM("Déconnexion...\n"); 

        }

    }

} 




void renderLOBBYClt(){
    // partie affichage
    BeginDrawing();

        ClearBackground(RAYWHITE);

        // bouton quitter
        DrawRectangle(760, 30, 30, 30, GRAY);
        DrawText("X", 770, 35, 20, BLACK);


        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            DrawRectangle(760, 30, 30, 30, RED);
            DrawText("X", 770, 35, 20, DARKGRAY);
        }

        // TODO : Voir pour mettre le pseudo dans une autre couleur
        DrawText(TextFormat("Bienvenue dans le serveur de [%s] !!", hote_serv_app.name), 10, 10, 20, BLACK);
        // affichage @IP + port
        //DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 20, 20, 20, BLACK);
        
        
        //for (int i=0; i<hotes.nbUsers; i++) {
        //    DrawText(TextFormat("> %s  - %s : %hu", hotes.tab[i].name, hotes.tab[i].adrIP, hotes.tab[i].port_srv_app), 40, 110+(20*i), 20, BLACK);
        //}


        DrawFPS(10, 450-20);

    EndDrawing();
}