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
#include "map.h"
#include "graphics.h"
#include "physic.h"


#define printIHM(fmt, ...) printf("\x1b[1;31mIHM (MAIN)\x1b[0m] " fmt, ##__VA_ARGS__)

#define envoi_avec_ack(cond_debut, cond_fin, mut_fin) pthread_cond_signal(&(cond_debut)); pthread_cond_wait(&(cond_fin), &(mut_fin));
#define envoi_no_ack(cond_debut) pthread_cond_signal(&(cond_debut));  // pas d'attente d'ACK, par exmeple pour le END_DIAL
#define estHote() (strcmp(hote_serv_app.name, pseudo) == 0) && (strcmp(hote_serv_app.adrIP, IP_SERVICE) == 0) && (hote_serv_app.port_srv_app == PORT_SRV_APP)


typedef enum {
    LIST = 1,
    LOBBY_HOTE, 
    LOBBY_CLIENT, 
    START,
    GAME, 
    END, 
    NEXT, 

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

void renderSTART();

void updateGAME();
void renderGAME(); 

void updateEND(); 
void renderEND(); 

void renderNEXT(); 


bool connecterClt2App(char * ip, short port);
void * requetes_recurrentes_app_1s(void * arg); 
void resetLIST(); 

void checkNbPlayers(); 

void charger_maps();


requete_t req_send_clt2reg;
pthread_cond_t end_reqrep_clt2reg = PTHREAD_COND_INITIALIZER;
pthread_cond_t start_reqrep_clt2reg = PTHREAD_COND_INITIALIZER;
pthread_mutex_t MUT_END_REQREP_CLT2REG = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUT_START_REQREP_CLT2REG = PTHREAD_MUTEX_INITIALIZER;

requete_t req_send_clt2app; 
pthread_cond_t end_reqrep_clt2app = PTHREAD_COND_INITIALIZER; 
pthread_cond_t start_reqrep_clt2app = PTHREAD_COND_INITIALIZER;
pthread_mutex_t MUT_END_REQREP_CLT2APP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUT_START_REQREP_CLT2APP = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t start_thread_clt2app = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t MUT_START_THREAD_CLT2APP = PTHREAD_MUTEX_INITIALIZER; 


pthread_cond_t cond_port_srv_app_alloue = PTHREAD_COND_INITIALIZER;
pthread_mutex_t MUT_COND_PORT_SRV_APP_ALLOUE = PTHREAD_MUTEX_INITIALIZER;

requete_t req_send_multi;
pthread_cond_t end_req_multitoclts = PTHREAD_COND_INITIALIZER;
pthread_cond_t start_req_multitoclts = PTHREAD_COND_INITIALIZER;
pthread_mutex_t MUT_END_REQ_MUTLITOCLTS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUT_START_REQ_MUTLITOCLTS = PTHREAD_MUTEX_INITIALIZER;

bool connexion_serv_reg_ok;
bool connexion_serv_app_ok;

bool deconnexion_serv_app = false; 
bool thread_app_running = false;
bool multicast_actif = false; 

bool start_game = false; 
bool end_game = false; 
bool next_player = false; 
bool next_round = false; 

int current_player_index; 

double startCountdownTime = 0;
double endScreenTime = 0;
double nextCountdownTime = 0; 

short PORT_SRV_REG;
short PORT_SRV_APP = 0;
char INTERFACE[50]; 
char IP_REG[100];
char IP_SERVICE[100];
char IP_MULTICAST[100] = "239.0.0.1";

char buff_pseudos_hotes[TAILLE_OPT];
char buff_info_joueur[TAILLE_OPT];
char buff_pseudos_players[TAILLE_OPT];
users_t hotes;
users_t clients_app; 
users_t clients; 

ball_t balls[MAX_USERS];

user_t hote_serv_app;  

socket_t sa_reg;
socket_t se;
socket_t sa;
socket_t sam;  // socket d'appel multicast
socket_t sm;  // socket multicast

game_state_t game_state; 
name_t pseudo;
name_t pseudo_next_player; 

map_t maps[MAX_MAPS];
int current_map = 0;


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

    sa_reg = connecterClt2Srv(IP_REG, PORT_SRV_REG);

    pthread_create(&th_clt2reg, NULL, (pFctThread)dialClt2Reg, (void*)&sa_reg);
    pthread_detach(th_clt2reg);
    pthread_create(&th_app_srv, NULL, serv_applicatif, NULL);
    pthread_detach(th_app_srv);


    pthread_cond_wait(&cond_port_srv_app_alloue, &MUT_COND_PORT_SRV_APP_ALLOUE);


    req_send_clt2reg.idReq=REG_PLAYER;
    strcpy(req_send_clt2reg.verbReq, "REG_PLAYER");
    sprintf(req_send_clt2reg.optReq, "%s:%s:%hu", pseudo, IP_SERVICE, PORT_SRV_APP);     

    connexion_serv_reg_ok = true;  // le thread de dialogue passera le flag à false si la connexion échoue

    envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);

    if (connexion_serv_reg_ok) {
        printIHM("Connexion au serveur d'enregistrement OK !\n");
    }
    else {
        printIHM("ERREUR: Connexion au serveur d'enregistrement échouée...\n");
        exit(EXIT_FAILURE);
    }


    // chargement de toutes les maps
    charger_maps();

    

    // création de l'IHM avec raylib
    SetTraceLogLevel(LOG_WARNING);  // on ne veut pas les messages d'info
    InitWindow(800, 450, TextFormat("IG2-GOLF - %s", pseudo));  // fenêtre 800 x 450 px

    // initialisation des assets/shaders
    init_graphics(maps);

    pthread_create(&th_req_rec, NULL, requetes_recurrentes_reg_1s, NULL);
    pthread_detach(th_req_rec);

    while (!WindowShouldClose()) {  // tant que la fenêtre ne doit pas se fermer


        if(start_game)
        {
            game_state = START;
            startCountdownTime = GetTime();
            start_game = false; 
        }
        else if (end_game)
        {
            game_state = END; 
            end_game = false; 
            endScreenTime = GetTime();
        }
        else if(next_round)
        {
            game_state = NEXT; 
            nextCountdownTime = GetTime();
            next_round = false; 
        }



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
        else if (game_state == START){
            renderSTART(); 
        }
        else if (game_state == GAME){
            updateGAME();
            renderGAME();  
        }
        else if (game_state == END){
            updateEND(); 
            renderEND();
        }
        else if (game_state == NEXT){
            renderNEXT(); 
        }

    }

    CloseWindow();

    // Déconnexion
    req_send_clt2reg.idReq=DIS_PLAYER;
    strcpy(req_send_clt2reg.verbReq, "DIS_PLAYER");
    strcpy(req_send_clt2reg.optReq, pseudo);
                
    envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);


    req_send_clt2reg.idReq=END_DIAL;
    strcpy(req_send_clt2reg.verbReq, "END_DIAL");
    strcpy(req_send_clt2reg.optReq, "");

    envoi_no_ack(start_reqrep_clt2reg);

    return 0;
}





void * serv_applicatif(void * arg) {

    socklen_t lenMyAddr = sizeof(se.addrLoc);
    printAppSrv("APP SERVER thread lance !\n");

    se = creerSocketEcoute("0.0.0.0", 0);

    CHECK(getsockname(se.fd, (struct sockaddr *)&se.addrLoc, &lenMyAddr),"--getsockname()--");    
    PORT_SRV_APP = ntohs(se.addrLoc.sin_port);

    sm = creerSocketMulti(IP_MULTICAST, PORT_SRV_APP);

    pthread_t th_multiSendToClts;

    pthread_create(&th_multiSendToClts, NULL, (pFctThread)multiSendToClts, (void*)&sm);
    pthread_detach(th_multiSendToClts);

    pthread_cond_signal(&cond_port_srv_app_alloue);  // adressage fini -> on signale au main

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

    while (1) {
        sleep(1);     
        
        if (game_state != LIST) continue; 

        req_send_clt2reg.idReq=GET_HOSTS_LIST;
        strcpy(req_send_clt2reg.verbReq, "GET_HOSTS_LIST");
        strcpy(req_send_clt2reg.optReq, "");
                    
        envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);



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

        for (int j=0; j<hotes.nbUsers; j++) {

            req_send_clt2reg.idReq=GET_PLAYER_FROM_ID;
            strcpy(req_send_clt2reg.verbReq, "GET_PLAYER_FROM_ID");
            strcpy(req_send_clt2reg.optReq, hotes.tab[j].name);
                        
            envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);

            sscanf(buff_info_joueur, "%c:%[^:]:%hu\n", &hotes.tab[j].etat, hotes.tab[j].adrIP, &hotes.tab[j].port_srv_app);

        }

    }

}


void updateLIST(){

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // bouton host
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 30, 30, 30})) {

            if (connecterClt2App(IP_SERVICE, PORT_SRV_APP)) {
                req_send_clt2reg.idReq=UPDT_CLIENT_STATE;
                strcpy(req_send_clt2reg.verbReq, "UPDT_CLIENT_STATE");
                snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:H", pseudo);
                            
                envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);

                strcpy(hote_serv_app.name, pseudo); 
                strcpy(hote_serv_app.adrIP, IP_SERVICE); 
                hote_serv_app.port_srv_app = PORT_SRV_APP; 
                hote_serv_app.etat = 'H'; 

                game_state = LOBBY_HOTE; 
            }
        }

        // bouton online
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){50, 30, 30, 30})) {

            req_send_clt2reg.idReq=UPDT_CLIENT_STATE;
            strcpy(req_send_clt2reg.verbReq, "UPDT_CLIENT_STATE");
            snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:O", pseudo);
                        
            envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);
        }

        // bouton full
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){90, 30, 30, 30})) {

            req_send_clt2reg.idReq=UPDT_CLIENT_STATE;
            strcpy(req_send_clt2reg.verbReq, "UPDT_CLIENT_STATE");
            snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:F", pseudo);
                        
            envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);
        }

        for (int i=0; i<hotes.nbUsers; i++) {

            requete_t req;

            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 110+(20*i), 20, 20})) {
                
                if (connecterClt2App(hotes.tab[i].adrIP, hotes.tab[i].port_srv_app)) {
                            
                    strcpy(hote_serv_app.name, hotes.tab[i].name); 
                    strcpy(hote_serv_app.adrIP, hotes.tab[i].adrIP); 
                    hote_serv_app.etat = hotes.tab[i].etat; 
                    hote_serv_app.port_srv_app = hotes.tab[i].port_srv_app; 

                    game_state = LOBBY_CLIENT; 
                }
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

void checkNbPlayers(){

    static int last_nb_clients = -1; 

    if (clients_app.nbUsers != last_nb_clients){
        last_nb_clients = clients_app.nbUsers; 

        req_send_clt2reg.idReq = UPDT_CLIENT_STATE; 
        strcpy(req_send_clt2reg.verbReq, "UPDT_CLIENT_STATE");

        if (clients_app.nbUsers >= NB_JOUEURS_MAX){
            snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:F", pseudo);
        }
        else{
            snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:H", pseudo);
        }

        envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);



    }

}


void updateLOBBY(){


    checkNbPlayers(); 

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        
        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {

            // Envoi message fermeture du serveur applicatif
            req_send_multi.idReq = END_SERV;
            strcpy(req_send_multi.verbReq, "END_SERV");
            strcpy(req_send_multi.optReq, "");

            envoi_avec_ack(start_req_multitoclts, end_req_multitoclts, MUT_END_REQ_MUTLITOCLTS);


            clients_app.nbUsers = 0; 

            // Déconnexion de son serveur
            req_send_clt2app.idReq = END_DIAL;
            strcpy(req_send_clt2app.verbReq, "END_DIAL");
            strcpy(req_send_clt2app.optReq, "");
            envoi_no_ack(start_reqrep_clt2app);


            req_send_clt2reg.idReq=UPDT_CLIENT_STATE;
            strcpy(req_send_clt2reg.verbReq, "UPDT_CLIENT_STATE");
            snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:O", pseudo);
                        
            envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);
            multicast_actif = false; 
            resetLIST(); 
            game_state = LIST; 

        }

        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){700, 430, 100, 20})) {
            req_send_multi.idReq = START_GAME;
            strcpy(req_send_multi.verbReq, "START_GAME");
            strcpy(req_send_multi.optReq, "");

            envoi_avec_ack(start_req_multitoclts, end_req_multitoclts, MUT_END_REQ_MUTLITOCLTS);


            // Fait disparaître le serveur applicatif de la liste des hotes
            req_send_clt2reg.idReq=UPDT_CLIENT_STATE;
            strcpy(req_send_clt2reg.verbReq, "UPDT_CLIENT_STATE");
            snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:F", pseudo);
                        
            envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);
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

        // bouton start
        DrawRectangle(700, 430, 100, 20, GRAY);
        DrawText("START", 720, 430, 20, BLACK);


        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            DrawRectangle(760, 30, 30, 30, RED);
            DrawText("X", 770, 35, 20, DARKGRAY);
        }
        // bouton start
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){700, 430, 100, 20})) {
            DrawRectangle(700, 430, 100, 20, GREEN);
            DrawText("START", 720, 430, 20, DARKGREEN);
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

    // Déconnexion du serveur applicatif 
    if (deconnexion_serv_app) {
        printIHM("... Le serveur applicatif vient de se fermer\n");

        // Fermeture propre du client avec le serveur applicatif
        
        req_send_clt2app.idReq = END_DIAL; 
        strcpy(req_send_clt2app.verbReq, "END_DIAL"); 
        strcpy(req_send_clt2app.optReq, ""); 

        envoi_no_ack(start_reqrep_clt2app);
        

        close(sam.fd); 

        multicast_actif = false; 

        resetLIST(); 
        game_state = LIST; 
        deconnexion_serv_app = false; 
        connexion_serv_app_ok = false; 

        return; 
    }

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        
        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            // Déconnexion du serveur applicatif
            printIHM("Déconnexion de la partie ...\n"); 

            req_send_clt2app.idReq=LEAVE_GAME; 
            strcpy(req_send_clt2app.verbReq, "LEAVE_GAME"); 
            strcpy(req_send_clt2app.optReq, pseudo); 

            envoi_no_ack(start_reqrep_clt2app); 

            multicast_actif = false; 
            resetLIST(); 
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
        
        
        for (int i=0; i<clients.nbUsers; i++) {
            DrawText(TextFormat("> %s ", clients.tab[i].name), 40, 110+(20*i), 20, BLACK);
        }


        DrawFPS(10, 450-20);

    EndDrawing();
}



bool connecterClt2App(char * ip, short port) {
    printIHM("Creation du thread de comm \"dialClt2App\" ...\n");

    pthread_t th_req_rec;
    pthread_t th_dialClt2App;
    pthread_t th_multiRecvFromApp;

    deconnexion_serv_app = false; 
    connexion_serv_app_ok = false;
    multicast_actif = false;

    sa = connecterClt2Srv(ip, port);

    pthread_create(&th_dialClt2App, NULL, (pFctThread)dialClt2App, (void*)&sa);
    pthread_detach(th_dialClt2App);

    pthread_cond_wait(&start_thread_clt2app, &MUT_START_THREAD_CLT2APP); 


    sam = connecterClt2Multi(IP_MULTICAST, port);
    multicast_actif = true; 

    pthread_create(&th_multiRecvFromApp, NULL, (pFctThread)multiRecvFromApp, (void*)&sam);
    pthread_detach(th_multiRecvFromApp);

    req_send_clt2app.idReq=JOIN_GAME; 
    strcpy(req_send_clt2app.verbReq, "JOIN_GAME"); 
    strcpy(req_send_clt2app.optReq, pseudo); 

    printIHM("...Attente de l'ack\n"); 

    connexion_serv_app_ok = true;  // le thread de dialogue le passera à false si la connexion échoue

    envoi_avec_ack(start_reqrep_clt2app, end_reqrep_clt2app, MUT_END_REQREP_CLT2APP); 

    if (connexion_serv_app_ok) {
        printIHM("... Connexion\n");

        if (!thread_app_running)
        {
            pthread_create(&th_req_rec, NULL, requetes_recurrentes_app_1s, NULL);
            pthread_detach(th_req_rec);
            thread_app_running = true;
        }

        return true;
    } else {
        printIHM("ERREUR: connexion refusée\n");
        return false;
    }  


}



void * requetes_recurrentes_app_1s(void * arg) {

    
    // partie envoie récurrents
    // contenu de la boucle exécuté une fois par seconde

    while (connexion_serv_app_ok) {
        sleep(1);        

        if(game_state != LOBBY_CLIENT) continue; 

        req_send_clt2app.idReq=GET_PLAYERS_LIST;
        strcpy(req_send_clt2app.verbReq, "GET_PLAYERS_LIST");
        strcpy(req_send_clt2app.optReq, "");
                    
        envoi_avec_ack(start_reqrep_clt2app, end_reqrep_clt2app, MUT_END_REQREP_CLT2APP);


        char copie_buff_pseudo_players[TAILLE_OPT];
        strcpy(copie_buff_pseudo_players, buff_pseudos_players);
        char * tok = strtok(copie_buff_pseudo_players, ":" );
        int i=0;
        while ( tok != NULL ) {
            strcpy(clients.tab[i].name, tok);
            i++;
            tok = strtok(NULL, ":" );
        }
        clients.nbUsers = i;
    }
    thread_app_running = false; 
    pthread_exit(EXIT_SUCCESS);

}


void resetLIST(){
    hotes.nbUsers = 0;
    for (int i = 0; i < MAX_USERS; i++)
        memset(hotes.tab[i].adrIP, 0, 16);
}


void renderSTART(){
    // partie affichage
    BeginDrawing();
        ClearBackground(RAYWHITE);

        double elapsed = GetTime() - startCountdownTime;
        int remaining = 3 - (int)elapsed;

        if (remaining > 0) {
            DrawText("Debut de la partie dans :", 220, 150, 30, BLACK);
            DrawText(TextFormat("%d", remaining), 380, 220, 60, RED);
        }
        else {
            game_state = GAME;
            startCountdownTime = 0; 
        }

        DrawFPS(10, 450-20);

    EndDrawing();

}


void updateGAME(){

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        
        // bouton fin de partie
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            // Déconnexion du serveur applicatif
            printIHM("Fin de la partie ...\n"); 

            req_send_multi.idReq = END_GAME;
            strcpy(req_send_multi.verbReq, "END_GAME");
            strcpy(req_send_multi.optReq, "");

            envoi_no_ack(start_req_multitoclts);
            end_game = true;
             
            game_state = END; 

            printIHM("Déconnexion...\n"); 

        }

        
        // bouton next player
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){20, 20, 20, 20})) {
            if (clients_app.nbUsers == 0) return;

            // Rotation circulaire
            current_player_index = (current_player_index + 1) % clients_app.nbUsers;

            req_send_multi.idReq = NEXT_PLAYER_TO_PLAY;
            strcpy(req_send_multi.verbReq, "NEXT_PLAYER_TO_PLAY");
            strcpy(req_send_multi.optReq, clients_app.tab[current_player_index].name);

            envoi_no_ack(start_req_multitoclts);
        }


        // bouton next round 
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){20, 60, 20, 20})) {

            req_send_multi.idReq = START_NEXT_ROUND;
            strcpy(req_send_multi.verbReq, "START_NEXT_ROUND");
            strcpy(req_send_multi.optReq, "");

            envoi_no_ack(start_req_multitoclts);
        }



    }

}

void renderGAME(){
    // partie affichage

    BeginDrawing();

        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

            render_current_map(maps, current_map);

        EndMode3D();

        if (estHote()){
            // bouton quitter
            DrawRectangle(760, 30, 30, 30, GRAY);
            DrawText("X", 770, 35, 20, BLACK);

            DrawRectangle(20, 20, 20, 20, GRAY);
            DrawText(">", 30, 25, 20, BLACK); 

            DrawRectangle(20, 60, 20, 20, GRAY); 
            DrawText("N", 30, 65, 20, BLACK);

            // récupération de la position de la souris
            int mouse_x = GetMouseX();
            int mouse_y = GetMouseY();

            // bouton fin de partie
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
                DrawRectangle(760, 30, 30, 30, RED);
                DrawText("X", 770, 35, 20, DARKGRAY);
            }

            // bouton next player
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){20, 20, 20, 20})) {
                DrawRectangle(20, 20, 20, 20, GREEN);
                DrawText(">", 30, 25, 20, DARKGRAY);
            }

            // bouton next round
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){20, 60, 20, 20})) {
                DrawRectangle(20, 60, 20, 20, GREEN);
                DrawText("N", 30, 65, 20, DARKGRAY);
            }
        }

        DrawText("La partie vient de commencer !!", 220, 150, 30, BLACK);

        if(next_player == true){
            if(strcmp(pseudo, pseudo_next_player) == 0){
                DrawText(TextFormat("C'est mon tour de jouer !"), 300, 10, 30, BLACK); 
            }
            else{
                DrawText(TextFormat("Joueur qui doit jouer : %s", pseudo_next_player), 300, 10, 30, BLACK); 
            }

        }


        DrawFPS(10, 450-20);

    EndDrawing();

}

void updateEND(){

    if (GetTime() - endScreenTime > 1) {

        if (estHote()){

            checkNbPlayers(); 

            game_state = LOBBY_HOTE;

        }
        else{
            game_state = LOBBY_CLIENT;
        }
 
    }

}


void renderEND(){

    // partie affichage
    BeginDrawing();

        ClearBackground(RAYWHITE);


        DrawText("La partie est terminée !!", 220, 150, 30, BLACK);


        DrawFPS(10, 450-20);

    EndDrawing();

}


void renderNEXT(){
    // partie affichage
    BeginDrawing();
        ClearBackground(RAYWHITE);

        double elapsed = GetTime() - nextCountdownTime;
        int remaining = 3 - (int)elapsed;

        if (remaining > 0) {
            DrawText("Prochaine manche dans :", 220, 150, 30, BLACK);
            DrawText(TextFormat("%d", remaining), 380, 220, 60, RED);
        }
        else {
            game_state = GAME;
            nextCountdownTime = 0; 
        }

        DrawFPS(10, 450-20);

    EndDrawing();
}







void charger_maps() {

    char buff[1000];

    for (int i=0; i<MAX_MAPS; i++) {
        snprintf(buff, 1000, "maps/map%d.txt", i);
        load_map(&maps[0], buff);
    }
}
