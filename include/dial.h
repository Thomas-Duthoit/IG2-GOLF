#ifndef DIAL_H
#define DIAL_H




// ---- SERVEUR D'ENREGISTREMENT | CLIENT -----------
void * dialReg2Clt(void * sd_p);
void * dialClt2Reg(void * sa_p);

// ---- SERVEUR D'APPLICATION | CLIENT -----------
void * dialApp2Clt(void * sd_p); 
void * dialClt2App(void * sa_p); 





#endif  // DIAL_H
