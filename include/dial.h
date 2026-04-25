#ifndef DIAL_H
#define DIAL_H

#define NB_JOUEURS_MAX 2


#ifdef DEBUG
    #define printClt2Reg(fmt, ...) printf("\x1b[1;32mDIAL REG\x1b[0m] " fmt, ##__VA_ARGS__)
#else
    #define printClt2Reg(fmt, ...)
#endif

#define printReg2Clt(fmt, ...) printf("\x1b[1;35mREGISTER SERVER\x1b[0m] " fmt, ##__VA_ARGS__)
#define printApp2Clt(fmt, ...) printf("\x1b[1;36mAPP SERVER (app2clt)\x1b[0m] " fmt, ##__VA_ARGS__)
#define printAppSrv(fmt, ...) printf("\x1b[1;36mAPP SERVER\x1b[0m] " fmt, ##__VA_ARGS__)
#define printClt2App(fmt, ...) printf("\x1b[1;36mAPP SERVER (clt2app)\x1b[0m] " fmt, ##__VA_ARGS__)
#define printMulticast(fmt, ...) printf("\x1b[1;37;41mMULTICAST\x1b[0m] " fmt, ##__VA_ARGS__)



// prototype fonction utilisées pour les threads
typedef void * (*pFctThread)(void *);


// ---- SERVEUR D'ENREGISTREMENT | CLIENT -----------
void * dialReg2Clt(socket_t * sd);
void * dialClt2Reg(socket_t * sa);

// ---- SERVEUR D'APPLICATION | CLIENT -----------
void * dialApp2Clt(socket_t * sd); 
void * dialClt2App(socket_t * sa); 

void * multiSendToClts(socket_t * sm);  // serv app -> multicast aux clients
void * multiRecvFromApp(socket_t * sam);  // réception des messages envoyés en multicast





#endif  // DIAL_H
