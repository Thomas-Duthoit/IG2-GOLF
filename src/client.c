/**
 * \file client.c
 * \brief Implémentation du client pour le jeu de golf multi-joueur
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

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


/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */


/**
 * \def APP_WIDTH
 * \brief Largeur de la fenêtre du client
 */
#define APP_WIDTH 800

/**
 * \def APP_HEIGHT
 * \brief Hauteur de la fenêtre du client
 */
#define APP_HEIGHT 450

/*
*****************************************************************************************
 *	\noop		PROTOTYPE DES FONCTIONS 
 */
/**
 * \fn void * serv_applicatif(void * arg)
 * \brief Fonction de thread pour la création et gestion du serveur applicatif et la communication avec lui
 * \param arg : Argument non utilisé lors de l'appel du thread
 */
void * serv_applicatif(void * arg);

/**
 * \fn void * requetes_recurrentes_reg_1s(void * arg)
 * \brief Fonction de thread pour les requêtes récurrentes vers le serveur d'enregistrement (toutes les secondes)
 * \param arg : Argument non utilisé lors de l'appel du thread
 */
void * requetes_recurrentes_reg_1s(void * arg);

/**
 * \fn void * requetes_recurrentes_app_1s(void * arg)
 * \brief Fonction de thread pour les requêtes récurrentes vers le serveur applicatif (toutes les secondes)
 * \param arg : Argument non utilisé lors de l'appel du thread
 */
void * requetes_recurrentes_app_1s(void * arg); 

/**
 * \fn void charger_maps()
 * \brief Charge les cartes du jeu
 */
void charger_maps();


/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    G L O B A L E S
 */

// Configuration réseau
/**
 * \var PORT_SRV_REG
 * \brief Port du serveur d'enregistrement (défini par l'argument de ligne de commande)
 */
short PORT_SRV_REG;
/**
 * \var PORT_SRV_APP
 * \brief Port du serveur applicatif (défini avec l'aide du serveur applicatif)
 */
short PORT_SRV_APP = 0;
/**
 * \var INTERFACE
 * \brief Interface réseau utilisée par le client (définie par l'argument de ligne de commande)
 */
char INTERFACE[50]; 
/**
 * \var IP_REG
 * \brief Adresse IP du serveur d'enregistrement (définie par l'argument de ligne de commande)
 */
char IP_REG[100];
/**
 * \var IP_SERVICE
 * \brief Adresse IP du client (définie avec l'aide du serveur d'enregistrement)
 */
char IP_SERVICE[100];
/**
 * \var IP_MULTICAST
 * \brief Adresse IP de multicast pour la communication entre clients lors d'une partie (définie par défaut)
 */
char IP_MULTICAST[100] = "239.0.0.1";

/**
 * \var cond_port_srv_app_alloue
 * \brief Condition pour signaler que le port du serveur applicatif a été alloué
 */
pthread_cond_t cond_port_srv_app_alloue = PTHREAD_COND_INITIALIZER;
/**
 * \var MUT_COND_PORT_SRV_APP_ALLOUE
 * \brief Mutex pour la condition que le port du serveur applicatif a été alloué
 */
pthread_mutex_t MUT_COND_PORT_SRV_APP_ALLOUE = PTHREAD_MUTEX_INITIALIZER;



// REQUETES / REPONSES
/**
 * \var req_send_clt2reg
 * \brief Requête envoyée du client vers le serveur d'enregistrement (définie au long du programme)
 */
requete_t req_send_clt2reg;
/**
 * \var end_reqrep_clt2reg
 * \brief Condition pour signaler la fin d'une requête vers le serveur d'enregistrement
 */
pthread_cond_t end_reqrep_clt2reg = PTHREAD_COND_INITIALIZER;
/**
 * \var start_reqrep_clt2reg
 * \brief Condition pour signaler le démarrage d'une requête vers le serveur d'enregistrement
 */
pthread_cond_t start_reqrep_clt2reg = PTHREAD_COND_INITIALIZER;
/**
 * \var MUT_END_REQREP_CLT2REG
 * \brief Mutex pour la condition de fin de requête vers le serveur d'enregistrement
 */
pthread_mutex_t MUT_END_REQREP_CLT2REG = PTHREAD_MUTEX_INITIALIZER;
/**
 * \var MUT_START_REQREP_CLT2REG
 * \brief Mutex pour la condition de démarrage de requête vers le serveur d'enregistrement
 */
pthread_mutex_t MUT_START_REQREP_CLT2REG = PTHREAD_MUTEX_INITIALIZER;


/**
 * \var req_send_clt2app
 * \brief Requête envoyée du client vers le serveur applicatif (définie au long du programme)
 */
requete_t req_send_clt2app; 
/**
 * \var end_reqrep_clt2app
 * \brief Condition pour signaler la fin d'une requête vers le serveur applicatif
 */
pthread_cond_t end_reqrep_clt2app = PTHREAD_COND_INITIALIZER; 
/**
 * \var start_reqrep_clt2app
 * \brief Condition pour signaler le démarrage d'une requête vers le serveur applicatif
 */
pthread_cond_t start_reqrep_clt2app = PTHREAD_COND_INITIALIZER;
/**
 * \var MUT_END_REQREP_CLT2APP
 * \brief Mutex pour la condition de fin de requête vers le serveur applicatif
 */
pthread_mutex_t MUT_END_REQREP_CLT2APP = PTHREAD_MUTEX_INITIALIZER;
/**
 * \var MUT_START_REQREP_CLT2APP
 * \brief Mutex pour la condition de démarrage de requête vers le serveur applicatif
 */
pthread_mutex_t MUT_START_REQREP_CLT2APP = PTHREAD_MUTEX_INITIALIZER;

/**
 * \var start_thread_clt2app
 * \brief Condition pour signaler le démarrage du thread de communication avec le serveur applicatif (début de dialogue entre un serveur applicatif et un client)
 */
pthread_cond_t start_thread_clt2app = PTHREAD_COND_INITIALIZER; 
/**
 * \var MUT_START_THREAD_CLT2APP
 * \brief Mutex pour la condition de démarrage du thread de communication avec le serveur applicatif (début de dialogue entre un serveur applicatif et un client)
 */
pthread_mutex_t MUT_START_THREAD_CLT2APP = PTHREAD_MUTEX_INITIALIZER; 

/**
 * \var req_send_multi
 * \brief Requête envoyée du client vers les autres clients via multicast (définie au long du programme)
 */
requete_t req_send_multi;
/**
 * \var end_req_multitoclts
 * \brief Condition pour signaler la fin d'une requête envoyée du serveur applicatif vers les autres clients via multicast
 */
pthread_cond_t end_req_multitoclts = PTHREAD_COND_INITIALIZER;
/**
 * \var start_req_multitoclts
 * \brief Condition pour signaler le démarrage d'une requête envoyée du serveur applicatif vers les autres clients via multicast
 */
pthread_cond_t start_req_multitoclts = PTHREAD_COND_INITIALIZER;
/**
 * \var MUT_END_REQ_MUTLITOCLTS
 * \brief Mutex pour la condition de fin de requête envoyée du serveur applicatif vers les autres clients via multicast
 */
pthread_mutex_t MUT_END_REQ_MUTLITOCLTS = PTHREAD_MUTEX_INITIALIZER;
/**
 * \var MUT_START_REQ_MUTLITOCLTS
 * \brief Mutex pour la condition de démarrage de requête envoyée du serveur applicatif vers les autres clients via multicast
 */
pthread_mutex_t MUT_START_REQ_MUTLITOCLTS = PTHREAD_MUTEX_INITIALIZER;


// CONNEXION / DECONNEXION
/**
 * \var connexion_serv_reg_ok
 * \brief Booléen indiquant si la connexion au serveur d'enregistrement est établie
 */
bool connexion_serv_reg_ok;
/**
 * \var connexion_serv_app_ok
 * \brief Booléen indiquant si la connexion au serveur applicatif est établie
 */
bool connexion_serv_app_ok;

/**
 * \var deconnexion_serv_app
 * \brief Booléen indiquant la fermeture du serveur applicatif
 */
bool deconnexion_serv_app = false;
/**
 * \var thread_app_running
 * \brief Booléen indiquant si le thread de communication avec le serveur applicatif est en cours d'exécution
 */
bool thread_app_running = false;
/**
 * \var multicast_actif
 * \brief Booléen indiquant si la communication multicast avec les autres clients est active quand le client est connecté à un serveur applicatif
 */
bool multicast_actif = false; 

// Partie jeu 
// flags de jeu
/**
 * \var start_game
 * \brief Booléen indiquant le début de la partie
 */
bool start_game = false; 
/**
 * \var end_game
 * \brief Booléen indiquant la fin de la partie
 */
bool end_game = false; 
/**
 * \var next_round
 * \brief Booléen indiquant le passage à la manche suivante
 */
bool next_round = false; 
/**
 * \var balls_initialized
 * \brief Booléen indiquant si les balles ont été initialisées
 */
bool balls_initialized = false; 
/**
 * \var next_player
 * \brief Booléen indiquant le passage au joueur suivant
 */
bool next_player = false; 

// Données de jeu
/**
 * \var current_player_index
 * \brief Index du joueur courant (celui qui doit jouer) dans la liste des clients (hôte ou clients_app selon le cas)
 */
int current_player_index; 
/**
 * \var compteur_podium
 * \brief Compteur pour le podium (nombre de manches déjà faites) 
 */
int compteur_podium = 0; 
/**
 * \var timePodium
 * \brief Temps de début d'affichage du podium (permet d'avoir un affichage constant du podium (10 secondes))
 */
double timePodium = 0; 
/**
 * \var startCountdownTime
 * \brief Temps de début d'affichage du début de partie (permet de faire un countdown de 3 secondes)
 */
double startCountdownTime = 0;
/**
 * \var endScreenTime
 * \brief Temps de début d'affiche de fin de partie 
 */
double endScreenTime = 0;
/**
 * \var nextCountdownTime
 * \brief Temps de début d'affichage de la manche suivante
 */
double nextCountdownTime = 0; 


// Données de l'application
/**
 * \var buff_pseudos_hotes
 * \brief Buffer pour stocker les pseudos des hôtes envoyés par le serveur d'enregistrement
 */
char buff_pseudos_hotes[TAILLE_OPT];
/**
 * \var buff_info_joueur
 * \brief Buffer pour stocker les informations du joueur envoyées par le serveur applicatif
 */
char buff_info_joueur[TAILLE_OPT];
/**
 * \var buff_pseudos_players
 * \brief Buffer pour stocker les pseudos des joueurs envoyés par le serveur applicatif
 */
char buff_pseudos_players[TAILLE_OPT];
/**
 * \var hotes
 * \brief Structure pour stocker la liste des hôtes disponibles (mise à jour par le serveur d'enregistrement)
 */
users_t hotes;
/**
 * \var clients_app
 * \brief Structure pour stocker la liste des clients connectés au serveur applicatif (mise à jour par le serveur applicatif)
 */
users_t clients_app; 
/**
 * \var clients
 * \brief Structure pour stocker la liste des clients connectés au serveur applicatif (mise à jour par le serveur applicatif, mais utilisée par les clients pour stocker la liste des joueurs de la partie)
 */
users_t clients; 

/**
 * \var balls
 * \brief Tableau pour stocker les informations des balles de tous les joueurs (position, vitesse, etc.)
 */
ball_t balls[MAX_USERS];
/**
 * \var my_ball_index
 * \brief Index de la balle du joueur dans le tableau balls (permet d'accéder facilement à sa balle)
 */
int my_ball_index = -1;  // -1 = pas trouvé, sinon index

/**
 * \var hote_serv_app
 * \brief Structure pour stocker les informations de l'hôte du serveur applicatif rejoint (adresse IP, pseudo, etc.)
 */
user_t hote_serv_app;  

// Sockets 
/**
 * \var sa_reg
 * \brief Socket de communication avec le serveur d'enregistrement
 */
socket_t sa_reg;
/**
 * \var se
 * \brief Socket d'écoute pour les requêtes entrantes (serveur applicatif)
 */
socket_t se;
/**
 * \var sa
 * \brief Socket d'appel du client vers un serveur applicatif
 */
socket_t sa;
//Multicast
/**
 * \var sam 
 * \brief Socket d'appel multicast client vers serveur applicatif pour la communication entre clients
 */
socket_t sam;  // socket d'appel multicast
/**
 * \var sm
 * \brief Socket de réception multicast du client pour la communication entre clients et serveur applicatif
 */
socket_t sm;  // socket multicast

/**
 * \var game_state
 * \brief État actuel du jeu (LIST, LOBBY_HOTE, LOBBY_CLIENT, START, GAME, END, NEXT ou PODIUM)
 */
game_state_t game_state; 
/**
 * \var pseudo
 * \brief Pseudo du joueur de l'application courante (défini par l'utilisateur à l'aide de la console au lancement du programme)
 */
name_t pseudo;
/**
 * \var pseudo_next_player
 * \brief Pseudo du prochain joueur à jouer (mis à jour par le serveur applicatif et utilisé pour indiquer au joueur quand c'est son tour de jouer) 
 */
name_t pseudo_next_player; 

/**
 * \var maps
 * \brief Tableau pour stocker les différentes cartes du jeu
 */
map_t maps[MAX_MAPS];
/**
 * \var current_map
 * \brief Index de la carte courante utilisée pour la partie en cours
 */
int current_map = 0;

/**
 * \var camera_mode
 * \brief Mode de la caméra (libre, à la première personne, etc.)
 */
cam_mode_t camera_mode = CAM_MODE_FREE;
/**
 * \var aiming
 * \brief Booléen indiquant si le joueur est en train de viser pour tirer
 */
bool aiming = false;  // en train de viser pour tirer
/**
 * \var shoot_puissance
 * \brief Puissance du tir en cours de préparation (calculée à partir de la distance du déplacement de la souris depuis le début du tir)
 */
float shoot_puissance = 0.0f;
/**
 * \var shoot_direction
 * \brief Direction du tir en cours de préparation (calculée à partir du déplacement de la souris depuis le début du tir)
 */
Vector3 shoot_direction = { 0 };
/**
 * \var mouse_delta
 * \brief Distance totale du déplacement de la souris depuis le début du tir (utilisée pour calculer la puissance et la direction du tir)
 */
Vector2 mouse_delta = { 0 };

/**
 * \var can_shoot
 * \brief Booléen indiquant si le joueur peut tirer ou non (permet de ne pas tirer si le joueur n'est pas dans la bonne vue ou s'il vient de tirer et doit attendre que la balle s'arrête)
 */
bool can_shoot;  // on peut tirer ou non

/**
 * \var set_ball_pos_envoye 
 * \brief Booléan indiquant si la position de la balle a été envoyé ou non 
 */
bool set_ball_pos_envoye = false;


// Podium
/**
 * \var scores
 * \brief Tableau pour stocker les scores des joueurs à chaque manche (utilisé pour le podium à la fin de la partie)
 */
int scores[NB_JOUEURS_MAX][NB_MANCHE]; 


/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */


/**
 * \fn int main(int argc, char **argv)
 * \brief Fonction principale du client
 * \param argc Nombre d'arguments de ligne de commande
 * \param argv Tableau des arguments de ligne de commande
 * \return Code de retour du programme
 */
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

    srand(time(NULL));  // seed aléatoire pour le random

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





/**
 * \fn void * serv_applicatif(void * arg)
 * \brief Fonction de thread pour la création et gestion du serveur applicatif et la communication avec lui
 * \param arg : Argument non utilisé lors de l'appel du thread
 */
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





/**
 * \fn void * requetes_recurrentes_reg_1s(void * arg)
 * \brief Fonction de thread pour les requêtes récurrentes vers le serveur d'enregistrement (toutes les secondes)
 * \param arg : Argument non utilisé lors de l'appel du thread
 */
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




/**
 * \fn void charger_maps()
 * \brief Charge les cartes du jeu depuis les fichiers
 */
void charger_maps() {

    char buff[1000];

    for (int i=0; i<MAX_MAPS; i++) {
        snprintf(buff, 1000, "maps/map%d.txt", i);
        load_map(&maps[i], buff);
    }
}






