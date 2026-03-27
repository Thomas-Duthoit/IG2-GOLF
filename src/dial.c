#include "dial.h"
#include "reqRep.h"
#include "pthread.h"
#include "string.h"
#include "users.h"


#define printClt2Reg(fmt, ...) printf("\x1b[1;32mTHREAD DIAL\x1b[0m] " fmt, ##__VA_ARGS__)
#define printReg2Clt(fmt, ...) printf("\x1b[1;35mREGISTER SERVER\x1b[0m] " fmt, ##__VA_ARGS__)


#ifdef CLIENT
    extern pthread_mutex_t MUT_CLT2REG;
    extern requete_t *req_send_clt2reg;
#endif

#ifdef SERVEUR
    extern pthread_mutex_t MUT_USER_MANAGEMENT; 
#endif


void traiterREG_PLAYER(requete_t * req, reponse_t * rep, socket_t * sd);
void traiterUPDT_CLIENT_STATE(requete_t * req, reponse_t * rep); 
void traiterGET_HOSTS_LIST(requete_t * req, reponse_t * rep); 
void traiterDIS_PLAYER(requete_t * req, reponse_t * rep);


void * dialReg2Clt(void * sd_p) {

    socket_t sd = *((socket_t*)sd_p);

    requete_t req;  
    reponse_t rep; 

    while (1) {
        
        recevoir(&sd, &req, (pFct)str2req);

        if (req.idReq==END_DIAL) {
            printReg2Clt("\x1b[1;31mEND_DIAL RECU\x1b[0m\n");
            getchar();
            break;
        }

        switch(req.idReq) {

            case REG_PLAYER : 
                
                // Demande d'enregistrement du joueur
                printReg2Clt("%s [%hu]\n", req.verbReq, req.idReq); 
                printReg2Clt("Options : %s\n", rep.optRep);
                traiterREG_PLAYER(&req, &rep, &sd); 

                break;
                
            case DIS_PLAYER : 

                // Déconnexion d'un joueur
                printReg2Clt("%s [%hu]\n", req.verbReq, req.idReq); 
                printReg2Clt("Options : %s\n", rep.optRep);
                traiterDIS_PLAYER(&req, &rep); 

                break;
            case JOIN_GAME : 
                break;
            case GET_HOSTS_LIST : 

                // Envoi de la liste des hôtes
                printReg2Clt("%s [%hu]\n", req.verbReq, req.idReq); 
                traiterGET_HOSTS_LIST(&req, &rep); 

                break;
            case UPDT_CLIENT_STATE : 
                
                // Changement de l'état du client 
                printReg2Clt("%s [%hu]\n", req.verbReq, req.idReq); 
                printReg2Clt("Options : %s\n", rep.optRep);
                traiterUPDT_CLIENT_STATE(&req, &rep); 

                break;
            case GET_PLAYER_FROM_ID : 
                break;

            default : 
                break; 
        }

        envoyer(&sd, &rep, (pFct)rep2str);

        strcpy(rep.optRep, ""); 
    } 
    
    CHECK(close(sd.fd), "--close()--");

    pthread_exit(EXIT_SUCCESS);
}


void * dialClt2Reg(void * sa_p) {

    socket_t sa = *((socket_t*)sa_p);

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

        printClt2Reg("%s [%hu]\n", req.verbReq, req.idReq);
        
        envoyer(&sa, &req, (pFct)req2str);

        printClt2Reg("Envoi fait\n");



        if(req.idReq == END_DIAL) {
            break;
        }

        if (  // TODO: mettre que les requetes où on attend une réponse
            req.idReq == REG_PLAYER
            || 1
        ) {


            printClt2Reg("Attente reception\n");

            recevoir(&sa, &rep, (pFct)str2rep);

            printClt2Reg("Reception OK\n");


            switch (rep.idRep)
            {
                case OK_REG_SERV:
                    printClt2Reg("Rep : OK_REG_SERV [%hu]\n", OK_REG_SERV);
                    break;
                case HOST_LIST:
                    printClt2Reg("Rep : HOST_LIST [%hu]\n", HOST_LIST);
                    printClt2Reg("Options : %s\n", rep.optRep);
                    printf("Liste des hotes :\n");
                    char * strToken = strtok ( rep.optRep, ":" );
                    while ( strToken != NULL ) {
                        printf ("\t> %s\n", strToken);
                        strToken = strtok ( NULL, ":" );
                    }
                    break;
                case PLAYER_DETAILS:
                    printClt2Reg("Rep : PLAYER_DETAILS [%hu]\n", PLAYER_DETAILS);
                    break;
                case ERR_REG_SERV:
                    printClt2Reg("Rep : ERR_REG_SERV [%hu]\n", ERR_REG_SERV);
                    break;
                    
                default:
                    break;
            }

            strcpy(rep.optRep, "");
            strcpy(rep.verbRep, "");
        }
        

    }
    CHECK(close(sa.fd), "--close-");

    pthread_exit(EXIT_SUCCESS);
}



void traiterREG_PLAYER(requete_t * req, reponse_t * rep, socket_t * sd) {
    name_t username; 
    strcpy(username, req->optReq); 
    int returnValue;

    printf("Demande d'enregistrement d'un joueur\n"); 

    pthread_mutex_lock(&MUT_USER_MANAGEMENT);
        returnValue = identifierUser(username, sd); 
    pthread_mutex_unlock(&MUT_USER_MANAGEMENT);

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
    printf("Modification état du joueurs\n"); 
    name_t username; 
    etat_joueur_t etat;
    int index = -1;  

    sscanf(req->optReq, "%[^:]:%c", username, (char*)&etat); 

    printf("%s => %c\n", username, etat); 

    pthread_mutex_lock(&MUT_USER_MANAGEMENT);
        index = trouverUser(username); 
    pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
    
    if(index != -1)
    {
        pthread_mutex_lock(&MUT_USER_MANAGEMENT);
            modifierEtat(index, etat); 
        pthread_mutex_unlock(&MUT_USER_MANAGEMENT);

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
    printf("Envoi de la liste des clients en état hôtes");
    char * listPseudo = (char *)malloc(sizeof(char)*(MAX_NAME+1)*MAX_USERS); 
    strcpy(listPseudo, "");

    pthread_mutex_lock(&MUT_USER_MANAGEMENT);
        getListPseudoByState(ETAT_HOST, listPseudo); 
    pthread_mutex_unlock(&MUT_USER_MANAGEMENT);

    printf("Liste des hôtes : %s\n", listPseudo); 
    
    rep->idRep = HOST_LIST; 
    strcpy(rep->optRep, listPseudo); 
    strcpy(rep->verbRep, "HOST_LIST"); 

    free(listPseudo); 
}

void traiterDIS_PLAYER(requete_t * req, reponse_t * rep){
    printf("Déconnexion du joueur\n"); 
    int index = -1;  

    name_t username; 
    sscanf(req->optReq, "%s", username); 

    printf("username : %s\n", username); 

    pthread_mutex_lock(&MUT_USER_MANAGEMENT);
        index = trouverUser(username); 
    pthread_mutex_unlock(&MUT_USER_MANAGEMENT);

    if (index != -1){
        pthread_mutex_lock(&MUT_USER_MANAGEMENT)
            deconnecterUser(index);
        pthread_mutex_unlock(&MUT_USER_MANAGEMENT);
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
























// void dialSrv2CltEcoute(socket_t sd) {

//     requete_t req;  
//     reponse_t rep;

//     while (1) {
//         //recevoir(sd, &req, (pFct)str2req);
//         switch(req.idReq) {
//             case JOIN_GAME : break;
//             case GET_PLAYERS_GAME : break;
//             case LEAVE_GAME : break;
//             case SHOOT : break;

//             default : 
//                 break; 
//         }
//         if (req.idReq==END_DIAL) {
//             break;
//         }
//     }

//     //envoyer(sd, rep, rep2str); 
//     CHECK(close(sd.fd), "--close()--");
// }

// void dialSrv2CltEnvoi(socket_t sd){

//     requete_t req;  
//     reponse_t rep;

//     while (1) {
//         //recevoir(sd, &req, (pFct)str2req);
//         switch(req.idReq) {

//             case UPDT_CLIENT_STATE : break;
//             case GET_PLAYER_FROM_ID : break;
//             case JOIN_GAME : break;
//             case GET_PLAYERS_GAME : break;
//             case LEAVE_GAME : break;
//             case SHOOT : break;
                        
//             default : 
//                 break; 
//         }
//         if (req.idReq==END_DIAL) {
//             break;
//         }
//     }

//     //envoyer(sd, rep, rep2str); 
//     CHECK(close(sd.fd), "--close()--");
// }




// void dialClt2SrvEcoute(socket_t sa){
//   requete_t req;
//   reponse_t rep;
//   while(1){
//     // IHM : menu pour interaction utilisateur final
//     // & création d'une requête
//     //envoyer(sa, &req, (pFct)req2str);
    
//     if(req.idReq == END_DIAL) {
//       break;
//     }


//     //switch (rep.idRep)
//     //{
//     //case constant expression:
//         /* code */
//     //    break;
    
//     //default:
//     //    break;
//     //}
    
//     //recevoir(sa, &rep, (pFct)strt2rep);
    
//     // traiter la réponse par le client


//   }
//   CHECK(close(sa.fd), "--close-");
// }


// void dialClt2SrvEnvoi(socket_t sa){
//   requete_t req;
//   reponse_t rep;
//   while(1){
//     // IHM : menu pour interaction utilisateur final
//     // & création d'une requête
//     //envoyer(sa, &req, (pFct)req2str);
    
//     if(req.idReq == END_DIAL) {
//       break;
//     }


//     //switch (rep.idRep)
//     //{
//     //case constant expression:
//         /* code */
//     //    break;
    
//     //default:
//     //    break;
//     //}
    
//     //recevoir(sa, &rep, (pFct)strt2rep);
    
//     // traiter la réponse par le client


//   }
//   CHECK(close(sa.fd), "--close-");
// }