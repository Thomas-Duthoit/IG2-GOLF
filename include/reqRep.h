#ifndef REQREP_H
#define REQREP_H

#define TAILLE_VERB 20
#define TAILLE_OPT 500


#define REQ_IN_OUT "%hu:%s:%s"          // req -> str
#define REQ_OUT_IN "%hu:%[^:]:%[^\n]"    // str -> req
#define REP_IN_OUT "%hu:%s:%s"          // rep -> str
#define REP_OUT_IN "%hu:%[^:]:%[^\n]"    // str -> rep


typedef struct {
    short idReq;
    char verbReq[TAILLE_VERB];
    char optReq[TAILLE_OPT];
} requete_t;

typedef struct {
    short idRep;
    char verbRep[TAILLE_VERB];
    char optRep[TAILLE_OPT];
} reponse_t;



typedef enum {

    END_DIAL = 0,

    // Requêtes Clients -> Serveur d’enregistrement
    REG_PLAYER = 001,
    DIS_PLAYER,
    GET_HOSTS_LIST,
    UPDT_CLIENT_STATE,
    GET_PLAYER_FROM_ID,

    // Requêtes Clients -> Serveur applicatif
    JOIN_GAME = 101,
    GET_PLAYERS_GAME,
    LEAVE_GAME,
    SHOOT,

    // Requêtes Serveur applicatif -> Clients
    START_GAME = 201,
    SET_BALL_VEL,
    SET_BALL_POS,
    NEXT_PLAYER_TO_PLAY,
    START_NEXT_ROUND,
    END_GAME,

    // Réponses Serveur d’enregistrement -> Clients
    OK_REG_SERV = 301,
    HOST_LIST,
    PLAYER_DETAILS, 
    ERR_REG_SERV, 

    // Réponses Serveur applicatif -> Clients
    OK_APP_SERV = 401, 
    NOK_APP_SERV

} listeReqRep_t;




// PROTOTYPES

// Pour les requêtes
void req2str(requete_t * req, char * str); 
void str2req(char * str, requete_t * req); 

// Pour les réponses
void rep2str(requete_t * rep, char * str); 
void str2rep(char * str, requete_t * rep);



#endif /* REQREP_H */