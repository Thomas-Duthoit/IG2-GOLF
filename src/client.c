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
#include "render.h"
#include "update.h"





#define APP_WIDTH 800
#define APP_HEIGHT 450




void * serv_applicatif(void * arg);
void * requetes_recurrentes_reg_1s(void * arg);
int get_local_ip(char *buffer);

void * requetes_recurrentes_app_1s(void * arg); 

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

bool balls_initialized = false; 

int current_player_index; 

int compteur_podium = 0; 
bool change_game_state = true; 
double timePodium = 0; 

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
int my_ball_index = -1;  // -1 = pas trouvé, sinon index

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

cam_mode_t camera_mode = CAM_MODE_FREE;
bool aiming = false;  // en train de viser pour tirer
float shoot_puissance = 0.0f;
Vector3 shoot_direction = { 0 };
Vector2 mouse_delta = { 0 };

bool can_shoot;  // on peut tirer ou non


bool set_ball_pos_envoye = false;


// Podium
int scores[NB_JOUEURS_MAX][NB_MANCHE]; 



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
    InitWindow(APP_WIDTH, APP_HEIGHT, TextFormat("IG2-GOLF - %s", pseudo));  // fenêtre 800 x 450 px

    // initialisation des assets/shaders
    init_graphics(maps);

    pthread_create(&th_req_rec, NULL, requetes_recurrentes_reg_1s, NULL);
    pthread_detach(th_req_rec);

    while (!WindowShouldClose()) {  // tant que la fenêtre ne doit pas se fermer


        if(start_game)
        {
            game_state = START;
            EnableCursor();
            startCountdownTime = GetTime();
            start_game = false; 
            balls_initialized = false; 
            compteur_podium = 0; 
            //shoot = true;  // A retirer
        }
        else if (end_game)
        {
            game_state = END; 
            EnableCursor();
            end_game = false; 
            endScreenTime = GetTime();
            balls_initialized = false; 
        }
        else if(next_round)
        {
            game_state = PODIUM; 
            EnableCursor();
            next_round = false; 
            balls_initialized = false; 
            timePodium = GetTime(); 
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
        else if (game_state == PODIUM){
            renderPODIUM();

            if (GetTime() - timePodium > 10) {  // durée du podium
                if(compteur_podium >= NB_MANCHE - 1){
                    game_state = END; 
                    endScreenTime = GetTime(); 
                }
                else {
                    game_state = NEXT;
                    nextCountdownTime = GetTime();
                }
                compteur_podium++; 
            }
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





















void charger_maps() {

    char buff[1000];

    for (int i=0; i<MAX_MAPS; i++) {
        snprintf(buff, 1000, "maps/map%d.txt", i);
        load_map(&maps[0], buff);
    }
}






