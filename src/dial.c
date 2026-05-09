/**
 * \file dial.c
 * \brief Gestion des dialogues réseau entre le client, le serveur d'enregistrement
 *        et le serveur applicatif (unicast et multicast)
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */


#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "users.h"
#include "data.h"
#include "dial.h"
#include "reqRep.h"
#include "physic.h"

/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    E X T E R N E S
 */


#ifdef CLIENT

    //#include "update.h"

    /**
     * \def NB_MANCHE
     * \brief Nombre de manche par partie
     */
    #define NB_MANCHE 3

    // Requêtes / réponses vers le serveur d'enregistrement
    extern requete_t req_send_clt2reg;
    extern char buff_pseudos_hotes[TAILLE_OPT];
    extern pthread_cond_t end_reqrep_clt2reg;
    extern pthread_cond_t start_reqrep_clt2reg;
    extern pthread_mutex_t MUT_START_REQREP_CLT2REG;
    extern char buff_info_joueur[TAILLE_OPT];

    // Liste des clients connectés au serveur applicatif
    extern users_t clients_app; 

    // Requêtes / réponses vers le serveur applicatif
    extern requete_t req_send_clt2app; 
    extern pthread_cond_t end_reqrep_clt2app; 
    extern pthread_cond_t start_reqrep_clt2app; 
    extern pthread_mutex_t MUT_START_REQREP_CLT2APP; 

    // Démarrage du thread de communication avec le serveur applicatif
    extern pthread_cond_t start_thread_clt2app; 
    extern pthread_mutex_t MUT_START_THREAD_CLT2APP; 

    // Requêtes multicast entre clients
    extern requete_t req_send_multi;
    extern pthread_cond_t end_req_multitoclts;
    extern pthread_cond_t start_req_multitoclts;
    extern pthread_mutex_t MUT_START_REQ_MUTLITOCLTS;

    // Flags de connexion
    extern bool connexion_serv_reg_ok;
    extern bool connexion_serv_app_ok;
    extern bool deconnexion_serv_app; 
    extern char buff_pseudos_players[TAILLE_OPT];
    extern bool multicast_actif; 

    // Flags de jeu
    extern bool start_game; 
    extern bool end_game; 
    extern bool next_round; 

    // Tour de jeu
    extern name_t pseudo_next_player; 
    extern bool next_player; 

    // Balles et joueurs
    extern ball_t balls[MAX_USERS];
    extern users_t clients;

    // Identité du joueur courant
    extern name_t pseudo;
    extern user_t hote_serv_app;  

    // Tir
    extern bool can_shoot;  // on peut tirer ou non
    extern bool set_ball_pos_envoye;

    // Podium
    extern int scores[NB_JOUEURS_MAX][NB_MANCHE]; // Podium
    extern int compteur_podium;

    // Carte courante
    extern int current_map;



    
#endif

#ifdef SERVER
    extern pthread_mutex_t MUT_USER_MANAGEMENT; 
#endif

#ifdef CLIENT
    /**
     * \def estHote()
     * \brief Renvoie vrai si le joueur courant est l'hôte du serveur applicatif
     */
    #define estHote() (strcmp(hote_serv_app.name, pseudo) == 0)
#endif


/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S   I N T E R N E S
 */

// Aiguillages
void switchReg2Clt(requete_t * req, reponse_t * rep, socket_t * sd); 
void switchClt2Reg(reponse_t rep);
void switchApp2Clt(requete_t * req, reponse_t * rep); 
void switchClt2App(requete_t * req, reponse_t * rep); 
void switchRecvFromApp(requete_t * req);

// Traitements serveur d'enregistrement
void traiterUPDT_CLIENT_STATE(requete_t * req, reponse_t * rep); 
void traiterGET_HOSTS_LIST(requete_t * req, reponse_t * rep); 
void traiterDIS_PLAYER(requete_t * req, reponse_t * rep);
void traiterGET_PLAYER_FROM_ID(requete_t * req, reponse_t * rep);
void traiterREG_PLAYER(requete_t * req, reponse_t * rep, socket_t * sd);

// Traitements serveur applicatif (requêtes client → serveur)
void traiterJOIN_GAME(requete_t * req, reponse_t * rep); 
void traiterGET_PLAYERS_LIST(requete_t * req, reponse_t * rep); 
void traiterLEAVE_GAME(requete_t * req, reponse_t * rep); 
void traiterSHOOT(requete_t * req, reponse_t * rep); 

// Traitements multicast (serveur applicatif → clients)
void traiterSTART_GAME(requete_t * req); 
void traiterSET_BALL_VEL(requete_t * req); 
void traiterSET_BALL_POS(requete_t * req); 
void traiterNEXT_PLAYER_TO_PLAY(requete_t * req); 
void traiterSTART_NEXT_ROUND(requete_t * req); 
void traiterEND_GAME(requete_t * req); 
void traiterEND_SERV(requete_t * req); 


/*
*****************************************************************************************
 *	\noop		IMPLEMENTATION   DES   FONCTIONS   I N T E R N E S
 */


//  --------------------------------------- SERVEUR D'ENREGISTREMENT | CLIENT -----------------------------

#pragma region DIALREG2CLT

/**
 * \fn void * dialReg2Clt(socket_t * sd)
 * \brief Fonction de thread gérant le dialogue du serveur d'enregistrement vers un client
 *        (réception des requêtes, aiguillage, envoi des réponses)
 * \param sd Pointeur vers la socket de dialogue avec le client
 * \return NULL
 */
void * dialReg2Clt(socket_t * sd) {

    requete_t req;  
    reponse_t rep; 

    while (1) {
        
        recevoir(sd, &req, (pFct)str2req);

        if (req.idReq==END_DIAL) {
            printReg2Clt("\x1b[1;31mEND_DIAL RECU\x1b[0m\n");
            break;
        }

        printReg2Clt("%s [%hu]\n", req.verbReq, req.idReq); 

        switchReg2Clt(&req, &rep, sd); 

        envoyer(sd, &rep, (pFct)rep2str);

        strcpy(rep.optRep, ""); 
    } 
    
    CHECK(close(sd->fd), "--close()--");

    pthread_exit(EXIT_SUCCESS);
}






    #pragma region SWITCH REG
// ------------------------------ PARTIE SWITCH -----------------------------------------

/**
 * \fn void switchReg2Clt(requete_t * req, reponse_t * rep, socket_t * sd)
 * \brief Aiguille la requête reçue du client vers le traitement correspondant (côté serveur d'enregistrement)
 * \param req Pointeur vers la requête reçue
 * \param rep Pointeur vers la réponse à remplir
 * \param sd  Pointeur vers la socket de dialogue
 */

void switchReg2Clt(requete_t * req, reponse_t * rep, socket_t * sd){
    switch(req->idReq) {

        case REG_PLAYER : 
            
            // Demande d'enregistrement du joueur
            printReg2Clt("Options : %s\n", req->optReq);
            traiterREG_PLAYER(req, rep, sd); 

            break;
            
        case DIS_PLAYER : 

            // Déconnexion d'un joueur
            printReg2Clt("Options : %s\n", req->optReq);
            traiterDIS_PLAYER(req, rep); 

            break;
        case GET_HOSTS_LIST : 

            // Envoi de la liste des hôtes
            traiterGET_HOSTS_LIST(req, rep); 

            break;
        case UPDT_CLIENT_STATE : 
            
            // Changement de l'état du client 
            printReg2Clt("Options : %s\n", req->optReq);
            traiterUPDT_CLIENT_STATE(req, rep); 

            break;
        case GET_PLAYER_FROM_ID : 
        
            // Changement de l'état du client 
            printReg2Clt("Options : %s\n", req->optReq);
            traiterGET_PLAYER_FROM_ID(req, rep); 
    
            break;

        default : 

            printReg2Clt("Options inconnues\n"); 
            break; 
    }
}

    #pragma endregion
// -------------------------------------------------------------------------------------------------------











    #pragma region TRAITEMENT REG
// ------------------------------ TRAITEMENT SERVEUR ENREGISTREMENT -----------------------------------------

/**
 * \fn void traiterREG_PLAYER(requete_t * req, reponse_t * rep, socket_t * sd)
 * \brief Traite la requête d'enregistrement d'un joueur (REG_PLAYER)
 * \param req Pointeur vers la requête contenant pseudo, IP et port
 * \param rep Pointeur vers la réponse (OK_REG_SERV ou ERR_REG_SERV)
 * \param sd  Pointeur vers la socket de dialogue
 */

void traiterREG_PLAYER(requete_t * req, reponse_t * rep, socket_t * sd) {
    name_t username; 
    //strcpy(username, req->optReq); 
    int returnValue;
    char * portEph = (char *)malloc(sizeof(char)*10); 
    short port; 
    char * adrIP = (char *)malloc(sizeof(char)*16); 
    
    printReg2Clt("Demande d'enregistrement d'un joueur\n"); 

    sscanf(req->optReq, "%[^:]:%[^:]:%[^\n]", username, adrIP, portEph); 

    port = atoi(portEph); 

    #ifdef SERVER
        pthread_mutex_lock(&MUT_USER_MANAGEMENT);

            returnValue = identifierUser(username, adrIP, port); 

        pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
    #endif 

    // Envoi de la réponse
    if  (returnValue != -1){
        rep->idRep = OK_REG_SERV; 
        strcpy(rep->optRep, ""); 
        strcpy(rep->verbRep, "OK_REG_SERV"); 
    }
    else{
        rep->idRep = ERR_REG_SERV; 
        strcpy(rep->optRep, ""); 
        strcpy(rep->verbRep, "ERR_REG_SERV"); 
    }
}


/**
 * \fn void traiterUPDT_CLIENT_STATE(requete_t * req, reponse_t * rep)
 * \brief Traite la requête de mise à jour de l'état d'un joueur (UPDT_CLIENT_STATE)
 * \param req Pointeur vers la requête contenant pseudo et nouvel état
 * \param rep Pointeur vers la réponse (OK_REG_SERV ou ERR_REG_SERV)
 */

void traiterUPDT_CLIENT_STATE(requete_t * req, reponse_t * rep){
    name_t username; 
    etat_joueur_t etat;
    int index = -1;  

    sscanf(req->optReq, "%[^:]:%c", username, (char*)&etat); 
 
    #ifdef SERVER
    pthread_mutex_lock(&MUT_USER_MANAGEMENT);
    
        index = trouverUser(username); 

    pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
    #endif

    // Envoi de la réponse
    if(index != -1)
    {
        #ifdef SERVER
        pthread_mutex_lock(&MUT_USER_MANAGEMENT);
            modifierEtat(index, etat); 
        pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
        #endif

        rep->idRep = OK_REG_SERV; 
        strcpy(rep->optRep, ""); 
        strcpy(rep->verbRep, "OK_REG_SERV"); 
        
    }
    else
    {

        rep->idRep = ERR_REG_SERV; 
        strcpy(rep->optRep, ""); 
        strcpy(rep->verbRep, "ERR_REG_SERV");  

    }

} 



/**
 * \fn void traiterGET_HOSTS_LIST(requete_t * req, reponse_t * rep)
 * \brief Traite la requête de récupération de la liste des hôtes disponibles (GET_HOSTS_LIST)
 * \param req Pointeur vers la requête (options non utilisées)
 * \param rep Pointeur vers la réponse contenant la liste des pseudos hôtes séparés par ':'
 */

void traiterGET_HOSTS_LIST(requete_t * req, reponse_t * rep){
    char * listPseudo = (char *)malloc(sizeof(char)*(MAX_NAME+1)*MAX_USERS); 
    strcpy(listPseudo, "");
    #ifdef SERVER
    pthread_mutex_lock(&MUT_USER_MANAGEMENT);
        getListPseudoByState(ETAT_HOST, listPseudo); 
    pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
    #endif
    
    // Envoi de la réponse
    rep->idRep = HOST_LIST; 
    strcpy(rep->optRep, listPseudo); 
    strcpy(rep->verbRep, "HOST_LIST"); 

    free(listPseudo); 
}



/**
 * \fn void traiterDIS_PLAYER(requete_t * req, reponse_t * rep)
 * \brief Traite la requête de déconnexion d'un joueur (DIS_PLAYER)
 * \param req Pointeur vers la requête contenant le pseudo du joueur
 * \param rep Pointeur vers la réponse (OK_REG_SERV ou ERR_REG_SERV)
 */
void traiterDIS_PLAYER(requete_t * req, reponse_t * rep){
    int index = -1;  

    name_t username; 
    sscanf(req->optReq, "%s", username);  

    #ifdef SERVER
        pthread_mutex_lock(&MUT_USER_MANAGEMENT);
            index = trouverUser(username); 
        pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
    #endif

    // Envoi de la réponse
    if (index != -1){
        #ifdef SERVER
            pthread_mutex_lock(&MUT_USER_MANAGEMENT); 
                deconnecterUser(index);
            pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
        #endif
        rep->idRep = OK_REG_SERV; 
        strcpy(rep->optRep, ""); 
        strcpy(rep->verbRep, "OK_REG_SERV");
    }
    else{
        rep->idRep = ERR_REG_SERV; 
        strcpy(rep->optRep, ""); 
        strcpy(rep->verbRep, "ERR_REG_SERV");
    }
}

/**
 * \fn void traiterGET_PLAYER_FROM_ID(requete_t * req, reponse_t * rep)
 * \brief Traite la requête de récupération des détails d'un joueur par son pseudo (GET_PLAYER_FROM_ID)
 * \param req Pointeur vers la requête contenant le pseudo du joueur
 * \param rep Pointeur vers la réponse contenant état, IP et port du joueur (ou ERR_REG_SERV)
 */

void traiterGET_PLAYER_FROM_ID(requete_t * req, reponse_t * rep){
    name_t pseudo; 
    int index; 

    char * detailsUser = (char *)malloc(sizeof(char)*TAILLE_OPT); 
    strcpy(detailsUser, "");

    sscanf(req->optReq, "%s", pseudo); 

    #ifdef SERVER 
        pthread_mutex_lock(&MUT_USER_MANAGEMENT);
            index = trouverUser(pseudo); 
        pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
    #endif
    
    // Envoi de la réponse
    if (index != -1){
        #ifdef SERVER
            pthread_mutex_lock(&MUT_USER_MANAGEMENT); 
                getDetailsUser(index, detailsUser); 
            pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
        #endif

        rep->idRep = PLAYER_DETAILS; 
        strcpy(rep->optRep, detailsUser); 
        strcpy(rep->verbRep, "PLAYER_DETAILS");
    }
    else{
        rep->idRep = ERR_REG_SERV; 
        strcpy(rep->optRep, ""); 
        strcpy(rep->verbRep, "ERR_REG_SERV");
    }

    free(detailsUser); 

}

    #pragma endregion
// -------------------------------------------------------------------------------------------------------

#pragma endregion










// -------------------------------------------------------------------------------------------------------

#pragma region DIALCLT2REG
/**
 * \fn void * dialClt2Reg(socket_t * sa)
 * \brief Fonction de thread gérant le dialogue du client vers le serveur d'enregistrement
 *        (attente de signal, envoi de requête, réception de réponse)
 * \param sa Pointeur vers la socket d'appel vers le serveur d'enregistrement
 * \return NULL
 */

void * dialClt2Reg(socket_t * sa) {

    requete_t req;
    reponse_t rep;
    while(1) {

        #ifdef CLIENT

        pthread_cond_wait(&start_reqrep_clt2reg, &MUT_START_REQREP_CLT2REG);

        req.idReq = req_send_clt2reg.idReq;
        strcpy(req.optReq, req_send_clt2reg.optReq);
        strcpy(req.verbReq, req_send_clt2reg.verbReq);
        
        #endif

        printClt2Reg("Req : %s [%hu]\n", req.verbReq, req.idReq);
        
        envoyer(sa, &req, (pFct)req2str);


        if(req.idReq == END_DIAL) {
            break;
        }
        recevoir(sa, &rep, (pFct)str2rep);


        switchClt2Reg(rep);

        strcpy(rep.optRep, "");
        strcpy(rep.verbRep, "");
        
        #ifdef CLIENT
        pthread_cond_signal(&end_reqrep_clt2reg);
        #endif

    }
    CHECK(close(sa->fd), "--close-");

    pthread_exit(EXIT_SUCCESS);
}




 #pragma region SWITCH CLT/REG
// ------------------------------ PARTIE SWITCH -----------------------------------------
/**
 * \fn void switchClt2Reg(reponse_t rep)
 * \brief Aiguille la réponse reçue du serveur d'enregistrement et met à jour les buffers client
 * \param rep Réponse reçue du serveur d'enregistrement
 */

void switchClt2Reg(reponse_t rep){


    switch (rep.idRep)
    {
        case OK_REG_SERV:
            printClt2Reg("Rep : OK_REG_SERV [%hu]\n", OK_REG_SERV);
            break;
        case HOST_LIST:
            printClt2Reg("Rep : HOST_LIST [%hu]\n", HOST_LIST);
            printClt2Reg("      Options : %s\n", rep.optRep);
            #ifdef CLIENT
            strcpy(buff_pseudos_hotes, rep.optRep);
            #endif
            break;
        case PLAYER_DETAILS:
            printClt2Reg("Rep : PLAYER_DETAILS [%hu]\n", PLAYER_DETAILS);
            printClt2Reg("      Options : %s\n", rep.optRep);
            #ifdef CLIENT
            strcpy(buff_info_joueur, rep.optRep);
            #endif
            break;
        case ERR_REG_SERV:
            printClt2Reg("Rep : ERR_REG_SERV [%hu]\n", ERR_REG_SERV);
            #ifdef CLIENT
            connexion_serv_reg_ok = 0;
            #endif
            break;
            
        default:
            break;
    }


}

    #pragma endregion
// -------------------------------------------------------------------------------------------------------





#pragma endregion

//  ------------------------------------------------------------------------------------------------------








//  --------------------------------------- SERVEUR D'APPLICATION | CLIENT -----------------------------

#pragma region DIALAPP2CLT
/**
 * \fn void * dialApp2Clt(socket_t * sd)
 * \brief Fonction de thread gérant le dialogue du serveur applicatif vers un client
 *        (réception des requêtes client, aiguillage, envoi des réponses)
 * \param sd Pointeur vers la socket de dialogue avec le client
 * \return NULL
 */

void * dialApp2Clt(socket_t * sd) {
    
    requete_t req;  
    reponse_t rep; 

    while (1) {
        
        recevoir(sd, &req, (pFct)str2req);
        printApp2Clt("Message reçu\n"); 

        if (req.idReq==END_DIAL) {
            printApp2Clt("\x1b[1;31mEND_DIAL RECU\x1b[0m\n");
            break;
        }

        printApp2Clt("%s [%hu]\n", req.verbReq, req.idReq); 


        switchApp2Clt(&req, &rep); 


        envoyer(sd, &rep, (pFct)rep2str);

        strcpy(rep.optRep, ""); 
    } 
    
    CHECK(close(sd->fd), "--close()--");

    pthread_exit(EXIT_SUCCESS);


}




 #pragma region SWITCH APP
// ------------------------------ PARTIE SWITCH -----------------------------------------
/**
 * \fn void switchApp2Clt(requete_t * req, reponse_t * rep)
 * \brief Aiguille la requête reçue d'un client vers le traitement correspondant (côté serveur applicatif)
 * \param req Pointeur vers la requête reçue
 * \param rep Pointeur vers la réponse à remplir
 */

void switchApp2Clt(requete_t * req, reponse_t * rep){


    switch(req->idReq) {

        case JOIN_GAME : 
            printApp2Clt("      Options : %s\n", rep->optRep);
            printApp2Clt("\n\nJOIN_GAME\n\n"); 
            traiterJOIN_GAME(req, rep); 
            break; 

        case GET_PLAYERS_LIST : 
            traiterGET_PLAYERS_LIST(req, rep); 
            break; 

        case LEAVE_GAME : 
            printApp2Clt(" Déconnexion !!!\n"); 
            printApp2Clt("      Options : %s\n", rep->optRep);
            traiterLEAVE_GAME(req, rep); 
            break; 


        case SHOOT : 
            printApp2Clt("      Options : %s\n", rep->optRep);
            traiterSHOOT(req, rep); 
            break; 


        default : 

            printApp2Clt("Options inconnues\n"); 
            break; 
    }


}

    #pragma endregion
// -------------------------------------------------------------------------------------------------------






// ------------------------------ TRAITEMENT APP / CLT -----------------------------------------

    #pragma region TRAITEMENT APP

/**
 * \fn void traiterJOIN_GAME(requete_t * req, reponse_t * rep)
 * \brief Traite la requête d'un joueur souhaitant rejoindre la partie (JOIN_GAME)
 * \param req Pointeur vers la requête contenant le pseudo du joueur
 * \param rep Pointeur vers la réponse (OK_APP_SERV si accepté, NOK_APP_SERV si partie pleine)
 */

void traiterJOIN_GAME(requete_t * req, reponse_t * rep){
    name_t username; 

    sscanf(req->optReq, "%s", username);

    printf("username : %s\n", username); 


    #ifdef CLIENT
        if (clients_app.nbUsers < NB_JOUEURS_MAX){

            strcpy(clients_app.tab[clients_app.nbUsers].name, username); 

            rep->idRep = OK_APP_SERV; 
            strcpy(rep->optRep, ""); 
            strcpy(rep->verbRep, "OK_APP_SERV");

            clients_app.nbUsers++; 

        }else{
            
            rep->idRep = NOK_APP_SERV; 
            strcpy(rep->optRep, ""); 
            strcpy(rep->verbRep, "NOK_APP_SERV");

        }

    #endif 

}


/**
 * \fn void traiterGET_PLAYERS_LIST(requete_t * req, reponse_t * rep)
 * \brief Traite la requête de récupération de la liste des joueurs de la partie (GET_PLAYERS_LIST)
 * \param req Pointeur vers la requête (options non utilisées)
 * \param rep Pointeur vers la réponse contenant la liste des pseudos séparés par ':'
 */

void traiterGET_PLAYERS_LIST(requete_t * req, reponse_t * rep){
    char * listClient = (char *)malloc(sizeof(char)*(MAX_NAME+1)*MAX_USERS);
    char * username = (char *)malloc(sizeof(char)*(MAX_NAME+1)); 
    int flag = 0; 
    strcpy(listClient, ""); 

    #ifdef CLIENT

        // Copie des pseudos de tous les joueurs
        for (int i = 0; i < clients_app.nbUsers; i++){
            strcpy(username, clients_app.tab[i].name); 
            strcat(username, ":"); 
            strcat(listClient, username); 
            flag = 1; 
        }

        if (flag){
            listClient[strlen(listClient)-1] = '\0'; 
        }

        rep->idRep = PLAYERS_LIST; 
        strcpy(rep->optRep, listClient); 
        strcpy(rep->verbRep, "PLAYERS_LIST");

    #endif

    free(username); 
    free(listClient); 


}






/**
 * \fn void traiterLEAVE_GAME(requete_t * req, reponse_t * rep)
 * \brief Traite la requête de départ d'un joueur de la partie (LEAVE_GAME)
 *        Décale les entrées suivantes dans le tableau pour combler le trou
 * \param req Pointeur vers la requête contenant le pseudo du joueur quittant
 * \param rep Pointeur vers la réponse (OK_APP_SERV ou NOK_APP_SERV)
 */
void traiterLEAVE_GAME(requete_t * req, reponse_t * rep){
    name_t username; 
    int flag = 0; 
    int i, j; 
    
    sscanf(req->optReq, "%s", username); 

    printf("Username : %s quitte\n", username);

    #ifdef CLIENT 
        // Vérifie que le joueur existe
        for (i = 0; i < clients_app.nbUsers; i++){
            if(strcmp(clients_app.tab[i].name, username) == 0){
                flag = 1; 
                continue; 
            }
        }


        if (flag == 1){
            // Supprime le joueur de la liste
            for (int j = i+1; j < clients_app.nbUsers; j++ ){
                strcpy(clients_app.tab[i].name, clients_app.tab[j].name); 
                i++; 
            }

            rep->idRep = OK_APP_SERV; 
            strcpy(rep->optRep, ""); 
            strcpy(rep->verbRep, "OK_APP_SERV");

            clients_app.nbUsers--; 

        }
        else{

            rep->idRep = NOK_APP_SERV; 
            strcpy(rep->optRep, ""); 
            strcpy(rep->verbRep, "NOK_APP_SERV");

        }

    #endif 


}


/**
 * \fn void traiterSHOOT(requete_t * req, reponse_t * rep)
 * \brief Traite la requête de tir d'un joueur (SHOOT)
 *        Applique la vélocité à la balle et diffuse la mise à jour en multicast
 * \param req Pointeur vers la requête contenant pseudo, direction (dx, dy, dz) et puissance
 * \param rep Pointeur vers la réponse (OK_APP_SERV si joueur trouvé)
 */

void traiterSHOOT(requete_t * req, reponse_t * rep){
    #ifdef CLIENT

        name_t name;
        float dx, dy, dz, p;

        
        sscanf(req->optReq, "%[^:]:%f:%f:%f:%f", name, &dx, &dy, &dz, &p);

        printApp2Clt("Tir reçu de %s (Power: %f)\n", name, p);

        // index joueur
        int index = -1;
        for (int i = 0; i < clients_app.nbUsers; i++) {
            if (strcmp(clients_app.tab[i].name, name) == 0) {
                index = i;
                break;
            }
        }

        set_ball_pos_envoye = false;

        // modif balle
        if (index != -1) {
            balls[index].vel.x = dx * p;
            balls[index].vel.y = dy * p;
            balls[index].vel.z = dz * p;
            balls[index].inMovement = true;

            rep->idRep = OK_APP_SERV;
            strcpy(rep->verbRep, "OK_APP_SERV");
            
            
            req_send_multi.idReq = SET_BALL_VEL;
            sprintf(req_send_multi.optReq, "%s:%f:%f:%f", name, balls[index].vel.x, balls[index].vel.y, balls[index].vel.z);

            pthread_cond_signal(&start_req_multitoclts);  // envoi no ack car en mode DGRAM
            
        }

    #endif 


}


    #pragma endregion
// -------------------------------------------------------------------------------------------------------



#pragma endregion








//  ------------------------------------------------------------------------------------------------------

#pragma region DIALCLT2APP

/**
 * \fn void * dialClt2App(socket_t * sa)
 * \brief Fonction de thread gérant le dialogue du client vers le serveur applicatif
 *        (attente de signal, envoi de requête, réception de réponse)
 * \param sa Pointeur vers la socket d'appel vers le serveur applicatif
 * \return NULL
 */

void * dialClt2App(socket_t * sa) {  // adrSrvApp = "port:IP"

    requete_t req;
    reponse_t rep;
    
    printf("THREAD dialClt2App lancé\n");

    #ifdef CLIENT
    pthread_cond_signal(&start_thread_clt2app); 
    #endif

    while(1) {


        #ifdef CLIENT
        pthread_cond_wait(&start_reqrep_clt2app, &MUT_START_REQREP_CLT2APP);

        req.idReq = req_send_clt2app.idReq;
        strcpy(req.optReq, req_send_clt2app.optReq);
        strcpy(req.verbReq, req_send_clt2app.verbReq);
        
        #endif


        printClt2Reg("Req : %s [%hu]\n", req.verbReq, req.idReq);
        
        envoyer(sa, &req, (pFct)req2str);



        if(req.idReq == END_DIAL) {
            break;
        }
        
        recevoir(sa, &rep, (pFct)str2rep);


        switchClt2App(&req, &rep); 


        strcpy(rep.optRep, "");
        strcpy(rep.verbRep, "");
        
        #ifdef CLIENT
        pthread_cond_signal(&end_reqrep_clt2app);
        #endif

    }
    CHECK(close(sa->fd), "--close-");

    pthread_exit(EXIT_SUCCESS);

}




 #pragma region SWITCH CLT2APP
// ------------------------------ PARTIE SWITCH -----------------------------------------
/**
 * \fn void switchClt2App(requete_t * req, reponse_t * rep)
 * \brief Aiguille la réponse reçue du serveur applicatif et met à jour les buffers client
 * \param req Pointeur vers la requête envoyée (non utilisée directement ici)
 * \param rep Pointeur vers la réponse reçue du serveur applicatif
 */

void switchClt2App(requete_t * req, reponse_t * rep){


    switch (rep->idRep)
    {
        case OK_APP_SERV:
            printClt2App("Rep : OK_APP_SERV [%hu]\n", OK_APP_SERV);
            break;
            
        case NOK_APP_SERV:
            printClt2App("Rep : NOK_APP_SERV [%hu]\n", NOK_APP_SERV);
            #ifdef CLIENT
            connexion_serv_app_ok = 0;
            #endif
            break;

        case PLAYERS_LIST : 
            printClt2App("Rep : OK_APP_SERV [%hu]\n", PLAYERS_LIST);
            printClt2Reg("      Options : %s\n", rep->optRep);

            #ifdef CLIENT
            strcpy(buff_pseudos_players, rep->optRep);
            #endif


            break; 

        default:
            break;
    }


}

    #pragma endregion
// -------------------------------------------------------------------------------------------------------










#pragma endregion

//  ------------------------------------------------------------------------------------------------------



//  --------------------------------------- SERVEUR D'APPLICATION | CLIENT (MULTICAST) -------------------


#pragma region MULTISENDTOCLTS
/**
 * \fn void * multiSendToClts(socket_t * sm) 
 * \brief Fonction de thread envoyant les requêtes de l'hôte aux autres clients en multicast
 *        (attente de signal, envoi UDP multicast, signal de fin si ACK attendu)
 * \param sm Pointeur vers la socket multicast d'envoi
 * \return NULL
 */

void * multiSendToClts(socket_t * sm) {
    requete_t req;
    
    printf("THREAD multiSendToClts lancé\n");

    while(1) {


        #ifdef CLIENT
        pthread_cond_wait(&start_req_multitoclts, &MUT_START_REQ_MUTLITOCLTS);

        req.idReq = req_send_multi.idReq;
        strcpy(req.optReq, req_send_multi.optReq);
        strcpy(req.verbReq, req_send_multi.verbReq);
        
        #endif
        printMulticast("Req : %s [%hu]\n", req.verbReq, req.idReq);
        
        // envoyer(sm, &req, (pFct)req2str);
        char *ip = inet_ntoa(sm->addrDst.sin_addr);
        int port = ntohs(sm->addrDst.sin_port);
        printMulticast("Multicast vers %s:%d\n", ip, port);
        envoyer(sm, &req, (pFct)req2str, ip, port);

        if(req.idReq == END_DIAL) {
            break;
        }
        #ifdef CLIENT
        pthread_cond_signal(&end_req_multitoclts);
        #endif

    }
    CHECK(close(sm->fd), "--close-");

    pthread_exit(EXIT_SUCCESS);
}

#pragma endregion



//  ------------------------------------------------------------------------------------------------------

#pragma region MULTIRECVFROMAPP
/**
 * \fn void * multiRecvFromApp(socket_t * sam)
 * \brief Fonction de thread recevant les messages multicast du serveur applicatif
 *        et les aiguillant vers le traitement correspondant
 * \param sam Pointeur vers la socket multicast de réception
 * \return NULL
 */

void * multiRecvFromApp(socket_t * sam) {
    requete_t req;  

    printMulticast("Thread multiRecvFromApp lancé !\n");

    while (1) {
        
        recevoir(sam, &req, (pFct)str2req);
        printMulticast("Message reçu\n"); 

        if (req.idReq==END_DIAL) {
            printMulticast("\x1b[1;31mEND_DIAL RECU\x1b[0m\n");
            break;
        }

        printMulticast("%s [%hu]\n", req.verbReq, req.idReq); 

        switchRecvFromApp(&req); 
    } 
    
    CHECK(close(sam->fd), "--close()--");

    pthread_exit(EXIT_SUCCESS);
}

#pragma endregion


//  ------------------------------------------------------------------------------------------------------

#pragma region SWITCHMULTICAST

/**
 * \fn void switchRecvFromApp(requete_t * req)
 * \brief Aiguille la requête multicast reçue du serveur applicatif vers le traitement correspondant
 * \param req Pointeur vers la requête multicast reçue
 */

void switchRecvFromApp(requete_t * req) {

    switch (req->idReq)
    {
        case START_GAME:
            traiterSTART_GAME(req); 

            break;

        case SET_BALL_VEL:
            traiterSET_BALL_VEL(req); 
            break;

        case SET_BALL_POS:
            traiterSET_BALL_POS(req); 
            break;

        case NEXT_PLAYER_TO_PLAY:
            traiterNEXT_PLAYER_TO_PLAY(req); 
            break;

        case START_NEXT_ROUND:
            traiterSTART_NEXT_ROUND(req); 
            break;

        case END_GAME:            
            traiterEND_GAME(req); 

            break;

        case END_SERV:             
            traiterEND_SERV(req); 


            break; 

        default:
            break;
    }


}


#pragma endregion


//  ------------------------------------------------------------------------------------------------------



// ------------------------------ TRAITEMENT APP / CLT MULTICAST -----------------------------------------

    #pragma region TRAITEMENT APP MULTICAST

/**
 * \fn void traiterSTART_GAME(requete_t * req)
 * \brief Traite le signal de démarrage de partie reçu en multicast (START_GAME)
 *        Met à jour l'index de la carte à jouer
 * \param req Pointeur vers la requête contenant l'index de la carte
 */

void traiterSTART_GAME(requete_t * req){

    #ifdef CLIENT 

        if(multicast_actif){

            start_game = true; 

            sscanf(req->optReq, "%d", &current_map);

        }
        else{
            printMulticast("START_GAME ignoré (session obsolète)\n"); 
        }


    #endif 

}

/**
 * \fn void traiterSET_BALL_VEL(requete_t * req)
 * \brief Traite la mise à jour de la vélocité d'une balle reçue en multicast (SET_BALL_VEL)
 *        Identifie la balle par le pseudo du joueur et applique la nouvelle vélocité
 * \param req Pointeur vers la requête contenant pseudo, vx, vy, vz
 */

void traiterSET_BALL_VEL(requete_t * req){

    #ifdef CLIENT

        if(multicast_actif){

            name_t name;
            float vx, vy, vz;

            
            sscanf(req->optReq, "%[^:]:%f:%f:%f", name, &vx, &vy, &vz);

            printMulticast("Nouvelle vel pour %s\n", name);

            // index joueur
            users_t *u_list;
            if (estHote()) {
                u_list = &clients_app;
            } else {
                u_list = &clients;
            }
            int nb_utilisateurs = u_list->nbUsers;

            // Trouver l'index 
            int index = -1;
            for (int i = 0; i < nb_utilisateurs; i++) {
                if (strcmp(u_list->tab[i].name, name) == 0) {
                    index = i;
                    break;
                }
            }

            // modif balle
            if (index != -1) {
                balls[index].vel.x = vx;
                balls[index].vel.y = vy;
                balls[index].vel.z = vz;
                balls[index].inMovement = true;
            }


        }else{

            printMulticast("SET_BALL_VEL ignoré (session obsolète)\n"); 

        }



    #endif 

}

/**
 * \fn void traiterSET_BALL_POS(requete_t * req)
 * \brief Traite la mise à jour de la position d'une balle reçue en multicast (SET_BALL_POS)
 *        Identifie la balle par le pseudo, applique la position et vérifie si toutes les balles sont dans le trou
 * \param req Pointeur vers la requête contenant pseudo, x, y, z et flag inHole
 */

void traiterSET_BALL_POS(requete_t * req){
    
    #ifdef CLIENT

        if(multicast_actif){

            name_t name;
            float x, y, z;
            bool inHole;
            int inHole_int; 

            
            sscanf(req->optReq, "%[^:]:%f:%f:%f:%d", name, &x, &y, &z, &inHole_int);
            inHole = (bool)inHole_int; 

            printMulticast("Nouvelle pos pour %s\n", name);
            printf("SET_BALL_POS reçu : nom=%s pos=%f %f %f inHole=%d\n", name, x, y, z, inHole);

            printf("optReq brut reçu : [%s]\n", req->optReq);

            // index joueur
            users_t *u_list;
            if (estHote()) {
                u_list = &clients_app;
            } else {
                u_list = &clients;
            }
            int nb_utilisateurs = u_list->nbUsers;

            // Trouver l'index 
            int index = -1;
            for (int i = 0; i < nb_utilisateurs; i++) {
                if (strcmp(u_list->tab[i].name, name) == 0) {
                    index = i;
                    break;
                }
            }

            // modif balle
            if (index != -1) {
                balls[index].pos.x = x;
                balls[index].pos.y = y;
                balls[index].pos.z = z;
                balls[index].inMovement = false;
                balls[index].inHole = inHole;


                bool all_in_hole = true; 

                for(int i = 0; i < u_list->nbUsers; i++){
                    if(!balls[i].inHole){
                        all_in_hole = false; 
                        break; 
                    }
                }

                if(all_in_hole && u_list->nbUsers > 0){
                    next_round = true; 
                }


            }


        }else{

            printMulticast("SET_BALL_POS ignoré (session obsolète)\n"); 

        }



    #endif 


}


/**
 * \fn void traiterNEXT_PLAYER_TO_PLAY(requete_t * req)
 * \brief Traite la notification du prochain joueur à jouer reçue en multicast (NEXT_PLAYER_TO_PLAY)
 *        Met à jour le pseudo du prochain joueur, autorise le tir si c'est notre tour et incrémente le score
 * \param req Pointeur vers la requête contenant le pseudo du prochain joueur
 */

void traiterNEXT_PLAYER_TO_PLAY(requete_t * req){
    
    #ifdef CLIENT 

        if(multicast_actif){
    
            sscanf(req->optReq, "%s", pseudo_next_player); 
            next_player = true; 
            if (!strcmp(pseudo, pseudo_next_player)) {
                can_shoot = true;
            }



            users_t *u_list;
            if (estHote()) {
                u_list = &clients_app;
            } else {
                u_list = &clients;
            }
            int nb_utilisateurs = u_list->nbUsers;

            int index = -1;
            for (int i = 0; i < nb_utilisateurs; i++) {
                if (strcmp(u_list->tab[i].name, pseudo_next_player) == 0) {
                    index = i;
                    break;
                }
            }

            printf("\n\n\n----------------------\nNEXT_PLAYER_TO_PLAY reçu : %s\n\n---------------------------\n\n\n", pseudo_next_player);
            
            scores[index][compteur_podium]++;

            

        }else{
            printMulticast("NEXT_PLAYER_TO_PLAY ignoré (session obsolète)\n"); 
        }


    #endif 
    
}

/**
 * \fn void traiterSTART_NEXT_ROUND(requete_t * req)
 * \brief Traite le signal de passage à la manche suivante reçu en multicast (START_NEXT_ROUND)
 *        Met à jour l'index de la prochaine carte
 * \param req Pointeur vers la requête contenant l'index de la carte suivante
 */

void traiterSTART_NEXT_ROUND(requete_t * req){

    #ifdef CLIENT 

        if(multicast_actif){

            next_round = true; 

            sscanf(req->optReq, "%d", &current_map);

        }else{

            printMulticast("START_NEXT_ROUND ignoré (session obsolète)\n"); 

        }


    #endif 
    
}

/**
 * \fn void traiterEND_GAME(requete_t * req)
 * \brief Traite le signal de fin de partie reçu en multicast (END_GAME)
 * \param req Pointeur vers la requête (options non utilisées)
 */

void traiterEND_GAME(requete_t * req){

    #ifdef CLIENT
        if(multicast_actif){
            end_game = true; 
        }
        else{
            printMulticast("END_SERV ignoré (session obsolète)\n"); 
        }
        

    #endif 

}

/**
 * \fn void traiterEND_SERV(requete_t * req)
 * \brief Traite le signal de fermeture du serveur applicatif reçu en multicast (END_SERV)
 * \param req Pointeur vers la requête (options non utilisées)
 */

void traiterEND_SERV(requete_t * req){

    #ifdef CLIENT
        if(multicast_actif){
            deconnexion_serv_app = true; 
        }
        else{
            printMulticast("END_SERV ignoré (session obsolète)\n"); 
        }
        

    #endif

}


    #pragma endregion
// -------------------------------------------------------------------------------------------------------
