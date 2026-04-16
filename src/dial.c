#include <string.h>
#include <pthread.h>
#include "users.h"
#include "data.h"
#include "dial.h"
#include "reqRep.h"


#ifdef CLIENT
    extern pthread_mutex_t MUT_CLT2REG;
    extern requete_t *req_send_clt2reg;
    extern short PORT_SRV_REG;
    extern char IP_REG[100];
    extern char buff_pseudos_hotes[TAILLE_OPT];
    extern pthread_cond_t end_reqrep_clt2reg;
    extern char buff_info_joueur[TAILLE_OPT];

    extern users_t clients_app; 

    extern pthread_mutex_t MUT_CLT2APP;
    extern requete_t *req_send_clt2app;
    extern pthread_cond_t end_reqrep_clt2app;
#endif

#ifdef SERVER
    extern pthread_mutex_t MUT_USER_MANAGEMENT; 
#endif



//TODO: Changer le prototype des fonct reponse_t * rep, socket_t * sd);
void traiterUPDT_CLIENT_STATE(requete_t * req, reponse_t * rep); 
void traiterGET_HOSTS_LIST(requete_t * req, reponse_t * rep); 
void traiterDIS_PLAYER(requete_t * req, reponse_t * rep);
void traiterGET_PLAYER_FROM_ID(requete_t * req, reponse_t * rep);
void traiterJOIN_GAME(requete_t * req, reponse_t * rep); 
void traiterREG_PLAYER(requete_t * req, reponse_t * rep, socket_t * sd);

//  --------------------------------------- SERVEUR D'ENREGISTREMENT | CLIENT -----------------------------

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

        switch(req.idReq) {

            case REG_PLAYER : 
                
                // Demande d'enregistrement du joueur
                printReg2Clt("Options : %s\n", req.optReq);
                traiterREG_PLAYER(&req, &rep, sd); 

                break;
                
            case DIS_PLAYER : 

                // Déconnexion d'un joueur
                printReg2Clt("Options : %s\n", req.optReq);
                traiterDIS_PLAYER(&req, &rep); 

                break;
            case JOIN_GAME : 
                break;
            case GET_HOSTS_LIST : 

                // Envoi de la liste des hôtes
                traiterGET_HOSTS_LIST(&req, &rep); 

                break;
            case UPDT_CLIENT_STATE : 
                
                // Changement de l'état du client 
                printReg2Clt("Options : %s\n", req.optReq);
                traiterUPDT_CLIENT_STATE(&req, &rep); 

                break;
            case GET_PLAYER_FROM_ID : 
            
                // Changement de l'état du client 
                printReg2Clt("Options : %s\n", req.optReq);
                traiterGET_PLAYER_FROM_ID(&req, &rep); 
            
                break;

            default : 

                printReg2Clt("Options inconnues\n"); 
                break; 
        }

        envoyer(sd, &rep, (pFct)rep2str);

        strcpy(rep.optRep, ""); 
    } 
    
    CHECK(close(sd->fd), "--close()--");

    pthread_exit(EXIT_SUCCESS);
}


void * dialClt2Reg(socket_t * sa) {

    requete_t req;
    reponse_t rep;
    while(1) {

        #ifdef CLIENT
        pthread_mutex_lock(&MUT_CLT2REG);

        if (req_send_clt2reg != NULL) {
            req.idReq = req_send_clt2reg->idReq;
            strcpy(req.optReq, req_send_clt2reg->optReq);
            strcpy(req.verbReq, req_send_clt2reg->verbReq);

            free(req_send_clt2reg);
            req_send_clt2reg = NULL;

            pthread_mutex_unlock(&MUT_CLT2REG);

        } else {
            pthread_mutex_unlock(&MUT_CLT2REG);
            continue;
        }

        #endif

        printClt2Reg("Req : %s [%hu]\n", req.verbReq, req.idReq);
        
        envoyer(sa, &req, (pFct)req2str);



        if(req.idReq == END_DIAL) {
            break;
        }
        recevoir(sa, &rep, (pFct)str2rep);


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
                // TODO: faire la gestion d'erreur
                break;
                
            default:
                break;
        }

        strcpy(rep.optRep, "");
        strcpy(rep.verbRep, "");
        
        #ifdef CLIENT
        pthread_cond_signal(&end_reqrep_clt2reg);
        #endif

    }
    CHECK(close(sa->fd), "--close-");

    pthread_exit(EXIT_SUCCESS);
}



// ------------------------------ TRAITEMENT SERVEUR ENREGISTREMENT -----------------------------------------


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


// -------------------------------------------------------------------------------------------------------

//  ------------------------------------------------------------------------------------------------------



//  --------------------------------------- SERVEUR D'APPLICATION | CLIENT -----------------------------


void * dialApp2Clt(socket_t * sd) {
    
    requete_t req;  
    reponse_t rep; 

    while (1) {
        
        recevoir(sd, &req, (pFct)str2req);

        if (req.idReq==END_DIAL) {
            printApp2Clt("\x1b[1;31mEND_DIAL RECU\x1b[0m\n");
            break;
        }

        printApp2Clt("%s [%hu]\n", req.verbReq, req.idReq); 

        switch(req.idReq) {

            case JOIN_GAME : 
                printApp2Clt("      Options : %s\n", rep.optRep);
                traiterJOIN_GAME(&req, &rep); 
                break; 

            case GET_PLAYERS_LIST : 
                printApp2Clt("      Options : %s\n", rep.optRep);
                break; 

            case LEAVE_GAME : 
                printApp2Clt("      Options : %s\n", rep.optRep);
                break; 


            case SHOOT : 
                printApp2Clt("      Options : %s\n", rep.optRep);
                break; 


            default : 

                printApp2Clt("Options inconnues\n"); 
                break; 
        }

        envoyer(sd, &rep, (pFct)rep2str);

        strcpy(rep.optRep, ""); 
    } 
    
    CHECK(close(sd->fd), "--close()--");

    pthread_exit(EXIT_SUCCESS);


}



void * dialClt2App(socket_t * sa) {  // adrSrvApp = "port:IP"

    requete_t req;
    reponse_t rep;
    while(1) {

        #ifdef CLIENT
        pthread_mutex_lock(&MUT_CLT2APP);

        if (req_send_clt2app != NULL) {
            req.idReq = req_send_clt2app->idReq;
            strcpy(req.optReq, req_send_clt2app->optReq);
            strcpy(req.verbReq, req_send_clt2app->verbReq);

            free(req_send_clt2app);
            req_send_clt2app = NULL;

            pthread_mutex_unlock(&MUT_CLT2APP);

        } else {
            pthread_mutex_unlock(&MUT_CLT2APP);
            continue;
        }

        #endif

        printClt2Reg("Req : %s [%hu]\n", req.verbReq, req.idReq);
        
        envoyer(sa, &req, (pFct)req2str);



        if(req.idReq == END_DIAL) {
            break;
        }
        
        recevoir(sa, &rep, (pFct)str2rep);


        switch (rep.idRep)
        {
            case OK_APP_SERV:
                printClt2App("Rep : OK_APP_SERV [%hu]\n", OK_APP_SERV);
                // TODO: faire la gestion des aquisitions
                break;
                
            case NOK_APP_SERV:
                printClt2App("Rep : NOK_APP_SERV [%hu]\n", NOK_APP_SERV);
                // TODO: faire la gestion des erreurs
                break;

            default:
                break;
        }

        strcpy(rep.optRep, "");
        strcpy(rep.verbRep, "");
        
        #ifdef CLIENT
        pthread_cond_signal(&end_reqrep_clt2app);
        #endif

    }
    CHECK(close(sa->fd), "--close-");

    pthread_exit(EXIT_SUCCESS);

}


void traiterJOIN_GAME(requete_t * req, reponse_t * rep){
    name_t username; 
    int i=0;

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







//  ------------------------------------------------------------------------------------------------------
