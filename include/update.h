#ifndef UPDATE_H
#define UPDATE_H

#define printIHM(fmt, ...) printf("\x1b[1;31mIHM (MAIN)\x1b[0m] " fmt, ##__VA_ARGS__)

#define envoi_avec_ack(cond_debut, cond_fin, mut_fin) pthread_cond_signal(&(cond_debut)); pthread_cond_wait(&(cond_fin), &(mut_fin));
#define envoi_no_ack(cond_debut) pthread_cond_signal(&(cond_debut));  // pas d'attente d'ACK, par exmeple pour le END_DIAL

#define estHote() (strcmp(hote_serv_app.name, pseudo) == 0) && (strcmp(hote_serv_app.adrIP, IP_SERVICE) == 0) && (hote_serv_app.port_srv_app == PORT_SRV_APP)


#define NB_MANCHE 3

typedef enum {
    LIST = 1,
    LOBBY_HOTE, 
    LOBBY_CLIENT, 
    START,
    GAME, 
    END, 
    NEXT, 
    PODIUM, 

} game_state_t; 

typedef enum {
    CAM_MODE_FREE,
    CAM_MODE_BALL,
} cam_mode_t;


void updateLIST(); 
void updateLOBBY(); 
void updateLOBBYClt(); 
void updateGAME();
void updateEND(); 


#endif  // UPDATE_H