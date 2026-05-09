/**
 * \file update.c
 * \brief Mise à jour de la logique de jeu selon les états
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#pragma region LIST

#include <pthread.h>

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
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    E X T E R N E S
 */

// Requêtes / réponses vers le serveur d'enregistrement
extern requete_t req_send_clt2reg;
extern pthread_cond_t end_reqrep_clt2reg;
extern pthread_cond_t start_reqrep_clt2reg;
extern pthread_mutex_t MUT_END_REQREP_CLT2REG;
extern pthread_mutex_t MUT_START_REQREP_CLT2REG;

// Requêtes / réponses vers le serveur applicatif
extern requete_t req_send_clt2app; 
extern pthread_cond_t end_reqrep_clt2app; 
extern pthread_cond_t start_reqrep_clt2app;
extern pthread_mutex_t MUT_END_REQREP_CLT2APP;
extern pthread_mutex_t MUT_START_REQREP_CLT2APP;

// Démarrage du thread de communication avec le serveur applicatif
extern pthread_cond_t start_thread_clt2app; 
extern pthread_mutex_t MUT_START_THREAD_CLT2APP; 

// Allocation du port du serveur applicatif
extern pthread_cond_t cond_port_srv_app_alloue;
extern pthread_mutex_t MUT_COND_PORT_SRV_APP_ALLOUE;

// Requêtes multicast entre clients
extern requete_t req_send_multi;
extern pthread_cond_t end_req_multitoclts;
extern pthread_cond_t start_req_multitoclts;
extern pthread_mutex_t MUT_END_REQ_MUTLITOCLTS;
extern pthread_mutex_t MUT_START_REQ_MUTLITOCLTS;

// Flags de connexion
extern bool connexion_serv_reg_ok;
extern bool connexion_serv_app_ok;
extern bool deconnexion_serv_app; 
extern bool thread_app_running;
extern bool multicast_actif; 

// Flags de jeu
extern bool start_game; 
extern bool end_game; 
extern bool next_player; 
extern bool next_round; 
extern bool balls_initialized; 

// Données de jeu
extern int current_player_index; 
extern int compteur_podium; 
extern bool change_game_state; 
extern double timePodium; 
extern double startCountdownTime;
extern double endScreenTime;
extern double nextCountdownTime; 

// Configuration réseau
extern short PORT_SRV_REG;
extern short PORT_SRV_APP;
extern char INTERFACE[50]; 
extern char IP_REG[100];
extern char IP_SERVICE[100];
extern char IP_MULTICAST[100];

// Données utilisateurs
extern char buff_pseudos_hotes[TAILLE_OPT];
extern char buff_info_joueur[TAILLE_OPT];
extern char buff_pseudos_players[TAILLE_OPT];
extern users_t hotes;
extern users_t clients_app; 
extern users_t clients; 

// Balles
extern ball_t balls[MAX_USERS];
extern int my_ball_index;  // -1 = pas trouvé, sinon index

// Hôte du serveur applicatif rejoint
extern user_t hote_serv_app;  

// Sockets
extern socket_t sa_reg;
extern socket_t se;
extern socket_t sa;
extern socket_t sam;  // socket d'appel multicast
extern socket_t sm;  // socket multicast

// État du jeu
extern game_state_t game_state; 
extern name_t pseudo;
extern name_t pseudo_next_player; 

// Cartes
extern map_t maps[MAX_MAPS];
extern int current_map;

// Caméra et tir
extern cam_mode_t camera_mode;
extern bool aiming;  // en train de viser pour tirer
extern float shoot_puissance;
extern Vector3 shoot_direction;
extern Vector2 mouse_delta;
extern bool can_shoot;  // on peut tirer ou non
extern bool set_ball_pos_envoye;

// Podium
extern int scores[NB_JOUEURS_MAX][NB_MANCHE]; // Podium


/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S   I N T E R N E S
 */

// Fonction interne 
void checkNbPlayers(); 
void resetLIST(); 
bool connecterClt2App(char * ip, short port); 
void * requetes_recurrentes_app_1s(void * arg); 
bool all_balls_in_hole(); 
int idx_my_ball(); 
void shoot(Vector3 dir, float power); 

/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

/**
 * \fn void updateLIST()
 * \brief Met à jour la logique de l'état LIST (liste des hôtes disponibles)
 */
void updateLIST(){

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        Rectangle hostButton = {300, 100, 200, 60};
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, hostButton)) {

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
                EnableCursor();
            }
        }

        for (int i=0; i<hotes.nbUsers; i++) {

            Rectangle hostEntry = {10, 230 + (30 * i), 300, 20};
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, hostEntry)) {
                
                if (connecterClt2App(hotes.tab[i].adrIP, hotes.tab[i].port_srv_app)) {
                            
                    strcpy(hote_serv_app.name, hotes.tab[i].name); 
                    strcpy(hote_serv_app.adrIP, hotes.tab[i].adrIP); 
                    hote_serv_app.etat = hotes.tab[i].etat; 
                    hote_serv_app.port_srv_app = hotes.tab[i].port_srv_app; 

                    game_state = LOBBY_CLIENT; 
                    EnableCursor();
                }
            }
        }
    }

}

#pragma endregion


#pragma region LOBBY

/**
 * \fn void updateLOBBY()
 * \brief Met à jour la logique de l'état LOBBY pour l'hôte (gestion des joueurs et démarrage de partie)
 */
void updateLOBBY(){

    checkNbPlayers(); 

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        
        Rectangle quitButton = {300, 100, 200, 60};
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, quitButton)) {

            req_send_multi.idReq = END_SERV;
            strcpy(req_send_multi.verbReq, "END_SERV");
            strcpy(req_send_multi.optReq, "");

            envoi_avec_ack(start_req_multitoclts, end_req_multitoclts, MUT_END_REQ_MUTLITOCLTS);

            clients_app.nbUsers = 0; 

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
            EnableCursor();

        }

        if(clients_app.nbUsers > 1){
            Rectangle startButton = {300, 180, 200, 60};
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, startButton)) {
                req_send_multi.idReq = START_GAME;
                strcpy(req_send_multi.verbReq, "START_GAME");
                int n = rand() % MAX_MAPS;
                snprintf(req_send_multi.optReq, TAILLE_OPT, "%d", n);

                envoi_avec_ack(start_req_multitoclts, end_req_multitoclts, MUT_END_REQ_MUTLITOCLTS);

                req_send_clt2reg.idReq=UPDT_CLIENT_STATE;
                strcpy(req_send_clt2reg.verbReq, "UPDT_CLIENT_STATE");
                snprintf(req_send_clt2reg.optReq, TAILLE_OPT, "%s:F", pseudo);
                            
                envoi_avec_ack(start_reqrep_clt2reg, end_reqrep_clt2reg, MUT_END_REQREP_CLT2REG);
            }
        }

    }

} 

#pragma endregion



#pragma region LOBBY Client

/**
 * \fn void updateLOBBYClt()
 * \brief Met à jour la logique de l'état LOBBY pour le client (gestion déconnexion et quitter)
 */
void updateLOBBYClt(){
    requete_t req; 

    if (deconnexion_serv_app) {
        printIHM("... Le serveur applicatif vient de se fermer\n");

        req_send_clt2app.idReq = END_DIAL; 
        strcpy(req_send_clt2app.verbReq, "END_DIAL"); 
        strcpy(req_send_clt2app.optReq, ""); 

        envoi_no_ack(start_reqrep_clt2app);
        

        close(sam.fd); 

        multicast_actif = false; 

        resetLIST(); 
        game_state = LIST; 
        EnableCursor();
        deconnexion_serv_app = false; 
        connexion_serv_app_ok = false; 

        return; 
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        
        Rectangle quitButton = {300, 100, 200, 60};
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, quitButton)) {
            printIHM("Déconnexion de la partie ...\n"); 

            req_send_clt2app.idReq=LEAVE_GAME; 
            strcpy(req_send_clt2app.verbReq, "LEAVE_GAME"); 
            strcpy(req_send_clt2app.optReq, pseudo); 

            envoi_no_ack(start_reqrep_clt2app); 

            multicast_actif = false; 
            resetLIST(); 
            game_state = LIST; 
            EnableCursor();

            printIHM("Déconnexion...\n"); 

        }

    }

} 


#pragma endregion



#pragma region GAME

/**
 * \fn void updateGAME()
 * \brief Met à jour la logique de l'état GAME (physique des balles, tir, passage au joueur suivant)
 */
void updateGAME(){

    int compteur = 0; 

    if (my_ball_index == -1) my_ball_index = idx_my_ball();

    ground_info_t ground_info; 
    int nb_joueurs = estHote() ? clients_app.nbUsers : clients.nbUsers; 

    if (!balls_initialized){
        if (estHote()) {
            current_player_index = 0;
            set_ball_pos_envoye = true; 

            req_send_multi.idReq = NEXT_PLAYER_TO_PLAY;
            strcpy(req_send_multi.verbReq, "NEXT_PLAYER_TO_PLAY");
            strcpy(req_send_multi.optReq, clients_app.tab[current_player_index].name);

            envoi_no_ack(start_req_multitoclts);
        }
        // Positionnement initial des balles 
        ground_info = get_ground_info(&maps[current_map], maps[current_map].start_x, maps[current_map].start_z); 
        printf("\n\n");

        for(int i = 0; i < nb_joueurs; i++){
            init_pos_ball(&(balls[i]), maps[current_map].start_x, maps[current_map].start_z, ground_info.y);
            printf("Balle [%d] : position : %f;%f;%f\n", i, balls[i].pos.x, balls[i].pos.y, balls[i].pos.z);
        }

        printf("\n\n");

        balls_initialized = true; 
    }


    if (estHote() && all_balls_in_hole()) {
        req_send_multi.idReq = START_NEXT_ROUND;
        strcpy(req_send_multi.verbReq, "START_NEXT_ROUND");
        //strcpy(req_send_multi.optReq, "");
        int n = rand() % MAX_MAPS;
        snprintf(req_send_multi.optReq, TAILLE_OPT, "%d", n);

        envoi_no_ack(start_req_multitoclts);

        next_round = true;
        set_ball_pos_envoye = false;
        balls_initialized = false; 
        return;
    }


    
    for(int i = 0; i < nb_joueurs; i++){
        double dt = GetFrameTime(); 
        if(!(balls[i].inHole)){
            update_ball_mov(&(balls[i]), dt, &maps[current_map]); 
            isInHole(&(balls[i]), &maps[current_map]); 
        }
    }



    if (estHote()){
        if (!set_ball_pos_envoye) {  // flag modifié dans traiterSHOOT
            int index = -1;
            for (int i = 0; i < clients_app.nbUsers; i++) {
                if (strcmp(clients_app.tab[i].name, pseudo_next_player) == 0) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                if (Vector3Length(balls[index].vel) < 0.01f) {

                    isInHole(&(balls[index]), &maps[current_map]); 

                    if (all_balls_in_hole()) {
                        req_send_multi.idReq = START_NEXT_ROUND;
                        strcpy(req_send_multi.verbReq, "START_NEXT_ROUND");
                        //strcpy(req_send_multi.optReq, "");
                        int n = rand() % MAX_MAPS;
                        snprintf(req_send_multi.optReq, TAILLE_OPT, "%d", n);
                    
                        envoi_no_ack(start_req_multitoclts);

                        next_round = true;
                        set_ball_pos_envoye = false;
                        balls_initialized = false;
                        return;
                    }

                    req_send_multi.idReq = SET_BALL_POS;
                    strcpy(req_send_multi.verbReq, "SET_BALL_POS");
                    snprintf(req_send_multi.optReq, TAILLE_OPT, "%s:%f:%f:%f:%d", 
                        clients_app.tab[current_player_index].name, 
                        balls[index].pos.x,
                        balls[index].pos.y,
                        balls[index].pos.z,
                        (int)balls[index].inHole
                    );

                    printApp2Clt("Envoi SET_BALL_POS avec options : %s:%f:%f:%f:%d\n", 
                        clients_app.tab[current_player_index].name, 
                        balls[index].pos.x,
                        balls[index].pos.y,
                        balls[index].pos.z,
                        (int)balls[index].inHole
                    );

                    envoi_avec_ack(start_req_multitoclts, end_req_multitoclts, MUT_END_REQ_MUTLITOCLTS);
                    
                    current_player_index = (current_player_index + 1) % clients_app.nbUsers;

                    if(!all_balls_in_hole() && balls[current_player_index].inHole){
                        while(balls[current_player_index].inHole != false){
                            current_player_index = (current_player_index + 1) % clients_app.nbUsers;
                        }
                    }

                    printf("\n\n");

                    printf("nb_joueurs=%d clients_app.nbUsers=%d\n", nb_joueurs, clients_app.nbUsers);
                    for(int i=0; i<clients_app.nbUsers; i++){
                        printf("clients_app.tab[%d].name=%s balls[%d].inHole=%d\n", i, clients_app.tab[i].name, i, balls[i].inHole);
                    }

                    printf("\n\n");

            
                    req_send_multi.idReq = NEXT_PLAYER_TO_PLAY;
                    strcpy(req_send_multi.verbReq, "NEXT_PLAYER_TO_PLAY");
                    strcpy(req_send_multi.optReq, clients_app.tab[current_player_index].name);

                    envoi_no_ack(start_req_multitoclts);

                    set_ball_pos_envoye = true;
                        
                }
            }
        }
    
    }   

    
    bool flag_bouton = false;  // on a appuyé sur un bouton de l'IHM -> pas de tir

    // partie IHM (clics, etc...)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // clic gauche

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        if (estHote()) {        
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
                EnableCursor();

                printIHM("Déconnexion...\n"); 

                flag_bouton = true;  // on a appuyé sur un bouton de l'IHM -> pas de tir

            }

            
            // bouton next round 
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){20, 60, 20, 20})) {

                req_send_multi.idReq = START_NEXT_ROUND;
                strcpy(req_send_multi.verbReq, "START_NEXT_ROUND");
                //strcpy(req_send_multi.optReq, "");
                int n = rand() % MAX_MAPS;
                snprintf(req_send_multi.optReq, TAILLE_OPT, "%d", n);

                envoi_no_ack(start_req_multitoclts);

                flag_bouton = true;  // on a appuyé sur un bouton de l'IHM -> pas de tir
            }

        }

        if (aiming) {
            flag_bouton = false;  // on était déja en train de tirer -> on reste en tir
        }

        if (!flag_bouton && camera_mode == CAM_MODE_BALL && can_shoot) {  // on a pas appuyé sur un bouton de l'IHM + vue balle -> ON TIRE
            aiming = true;  // passage en mode tir
        }

    }


    if (aiming) {
        Vector2 current_delta = GetMouseDelta();
        
        mouse_delta.x += current_delta.x;
        mouse_delta.y += current_delta.y;

        shoot_puissance = Vector2Length((Vector2){mouse_delta.x, mouse_delta.y}) * 0.1f; 
        if (shoot_puissance > MAX_PUISSANCE) {
            shoot_puissance = MAX_PUISSANCE;
        }

        
        Vector3 cam_forward = Vector3Subtract(camera.target, camera.position);
        cam_forward.y = 0; // pas de hauteur, plan horizontal seulement
        cam_forward = Vector3Normalize(cam_forward);
        
        
        // on ajoute l'écart du drag à la direction de la caméra pour tirer sur les côtés
        Vector3 cam_right = { -cam_forward.z, 0, cam_forward.x };
        
        shoot_direction = Vector3Add(
            Vector3Scale(cam_forward, mouse_delta.y * 0.01f), 
            Vector3Scale(cam_right, mouse_delta.x * 0.01f)
        );
        shoot_direction = Vector3Normalize(shoot_direction);

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            aiming = false;
            mouse_delta = (Vector2){0, 0};
            shoot(shoot_direction, shoot_puissance);

            can_shoot = false;
            
        }
    }

    cam_mode_t old_mode = camera_mode;

    if (!aiming) {  // camera bloquée en tir
        // gestion camera : zqsd espace ctrl -> mode libre, r -> mode balle
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D) || 
            IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_LEFT_CONTROL)) 
        {
            camera_mode = CAM_MODE_FREE;
        }
        
        if (IsKeyPressed(KEY_R)) {
            camera_mode = CAM_MODE_BALL;
        }

        if (camera_mode == CAM_MODE_FREE) {
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                EnableCursor();  // shift : curseur
            if (IsKeyReleased(KEY_LEFT_SHIFT)) {
                DisableCursor();
            }
            } else {
                if (old_mode != camera_mode) {
                    DisableCursor(); // pas shift : pas curseur
                }
            }
        }

        old_mode = camera_mode;

        if (camera_mode == CAM_MODE_FREE) {
            UpdateCamera(&camera, CAMERA_FREE);
        } 
        else if (camera_mode == CAM_MODE_BALL && my_ball_index != -1) {
            camera.target = balls[my_ball_index].pos;
            UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        }
    }

}

#pragma endregion


#pragma region END

/**
 * \fn void updateEND()
 * \brief Met à jour la logique de l'état END (retour au lobby après la fin de partie)
 */
void updateEND(){

    if (GetTime() - endScreenTime > 1) {

        if (estHote()){

            checkNbPlayers(); 

            game_state = LOBBY_HOTE;
            EnableCursor();

        }
        else{
            game_state = LOBBY_CLIENT;
            EnableCursor();

        }
 
    }

}

#pragma endregion


#pragma region FONCTION INTERNE


/**
 * \fn void checkNbPlayers()
 * \brief Vérifie le nombre de joueurs connectés et met à jour l'état de l'hôte auprès du serveur d'enregistrement
 */
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




/**
 * \fn void resetLIST() 
 * \brief Réinitialise la liste des hôtes disponibles
 */
void resetLIST(){
    hotes.nbUsers = 0;
    for (int i = 0; i < MAX_USERS; i++)
        memset(hotes.tab[i].adrIP, 0, 16);
}



/**
 * \fn bool connecterClt2App(char * ip, short port)
 * \brief Connecte le client à un serveur applicatif et démarre les threads de communication associés
 * \param ip   Adresse IP du serveur applicatif
 * \param port Port du serveur applicatif
 * \return true si la connexion a réussi, false sinon
 */
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




/**
 * \fn void * requetes_recurrentes_app_1s(void * arg)
 * \brief Fonction de thread pour les requêtes récurrentes vers le serveur applicatif (toutes les secondes)
 * \param arg Argument non utilisé lors de l'appel du thread
 * \return NULL
 */
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


/**
 * \fn bool all_balls_in_hole()
 * \brief Vérifie si toutes les balles des joueurs sont dans le trou
 * \return true si toutes les balles sont dans le trou, false sinon
 */
bool all_balls_in_hole(){
    int nbUsers = estHote() ? clients_app.nbUsers : clients.nbUsers; 

    if(nbUsers == 0) return false; 

    for(int i = 0; i < nbUsers; i++){
        if(!balls[i].inHole) {
            return false; 
        }
    }
    return true; 
}


/**
 * \fn int idx_my_ball()
 * \brief Recherche l'index de la balle du joueur courant dans le tableau des balles
 * \return Index de la balle dans le tableau balls, ou -1 si non trouvé
 */
int idx_my_ball() {
    if (estHote()) {
        for (int i = 0; i < clients_app.nbUsers; i++) {
            if (strcmp(clients_app.tab[i].name, pseudo) == 0) return i;
        }
    } else {
        for (int i = 0; i < clients.nbUsers; i++) {
            if (strcmp(clients.tab[i].name, pseudo) == 0) return i;
        }
    }
    return -1;
}




/**
 * \fn void shoot (Vector3 dir, float power)
 * \brief Envoie un tir au serveur applicatif
 * \param dir   Direction du tir (vecteur normalisé)
 * \param power Puissance du tir
 */
void shoot(Vector3 dir, float power) {
    if (!connexion_serv_app_ok) return;

    req_send_clt2app.idReq = SHOOT;
    strcpy(req_send_clt2app.verbReq, "SHOOT");
    
    snprintf(req_send_clt2app.optReq, TAILLE_OPT, "%s:%f:%f:%f:%f", 
             pseudo, dir.x, dir.y, dir.z, power);

    printIHM("Envoi du tir : %s\n", req_send_clt2app.optReq);

    envoi_avec_ack(start_reqrep_clt2app, end_reqrep_clt2app, MUT_END_REQREP_CLT2APP);
}


#pragma endregion