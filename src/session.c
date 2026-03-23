/**
 *	\file		session.c
 *	\brief		Code de la couche Session
 *	\author		Thomas DUTHOIT
 *	\date		10 mars 2026
 *	\version	1.0
 */
#include <unistd.h> // POSIX
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "session.h"

/**
 *	\fn			void adr2struct (struct sockaddr_in *addr, char *adrIP, short port)
 *	\brief		Transformer une adresse au format humain en structure SocketBSD
 *	\param		addr : structure d'adressage BSD d'une socket INET
 *	\param		adrIP : adresse IP de la socket créée
 *	\param		port : port de la socket créée
 *	\note		Le domaine dépend du mode choisi (TCP/UDP)
 *	\result		paramètre *adr modifié
 */
void adr2struct (struct sockaddr_in *addr, char *adrIP, short port) {
    addr->sin_family = PF_INET;
    // adrSrv.sin_addr.s_addr = INADDR_ANY;
    // 0.0.0.0 : toute interface rézo
    addr->sin_addr.s_addr=inet_addr(adrIP);//network order
    addr->sin_port = htons(port); // network order
    memset(&(addr->sin_zero), 0, 8);

}
/**
 *	\fn			socket_t creerSocket (int mode)
 *	\brief		Création d'une socket de type DGRAM/STREAM
 *	\param		mode : mode connecté (STREAM) ou non (DGRAM)
 *	\result		socket créée selon le mode choisi
 */
socket_t creerSocket (int mode) {
    socket_t sock;
    CHECK(sock.fd = socket(PF_INET, mode, 0), "__socket__");
    PAUSE("socket créée");

    sock.mode = mode;

    return sock;
}
/**
 *	\fn			socket_t creerSocketAdr (int mode, char *adrIP, short port)
 *	\brief		Création d'une socket de type DGRAM/STREAM
 *	\param		mode : adresse IP de la socket créée
 *	\param		adrIP : adresse IP de la socket créée
 *	\param		port : port de la socket créée
 *	\result		socket créée dans le domaine choisi avec l'adressage fourni
 */
socket_t creerSocketAdr (int mode, char *adrIP, short port) {
    socket_t sock = creerSocket(mode);

    adr2struct(&sock.addrLoc, adrIP, port);
    CHECK(bind(sock.fd,(struct sockaddr *)&sock.addrLoc, sizeof(sock.addrLoc)),"__bind__");
    PAUSE("socket créée et adressée");

    return sock;
}
/**
 *	\fn			creerSocketEcoute (char *adrIP, short port)
 *	\brief		Création d'une socket d'écoute avec l'adressage fourni en paramètre
 *	\param		adrIP : adresse IP du serveur à mettre en écoute
 *	\param		port : port TCP du serveur à mettre en écoute
 *	\result		socket créée avec l'adressage fourni en paramètre et dans un état d'écoute
 *	\note		Le domaine est nécessairement STREAM
 */
socket_t creerSocketEcoute (char *adrIP, short port) {

    socket_t sock = creerSocketAdr(SOCK_STREAM, adrIP, port);

    CHECK (listen(sock.fd, 5), "-- PB listen() --");
    PAUSE("SERVER-après listen()"); // observation netstat / listen()
    printf("Serveur [\033[32m%d\033[0m] : attente de connexion sur [\033[32m%s:%d\033[0m]\n",
        getpid(), 
        inet_ntoa(sock.addrLoc.sin_addr),
        ntohs(sock.addrLoc.sin_port)
    );

    return sock;
}
/**
 *	\fn			socket_t accepterClt (const socket_t sockEcoute)
 *	\brief		Acceptation d'une demande de connexion d'un client
 *	\param		sockEcoute : socket d'écoute pour réception de la demande
 *	\result		socket (dialogue) connectée par le serveur avec un client
 */
socket_t accepterClt (const socket_t sockEcoute) {

    socket_t sd;
    socklen_t lenAdrClt = sizeof(sd.addrDst);

    PAUSE("SERVER-avant accept()"); // observation netstat / accept()
    CHECK(sd.fd=accept(sockEcoute.fd,(struct sockaddr *)&sd.addrDst, &lenAdrClt), "--accept()--");

    sd.mode = sockEcoute.mode;
    sd.addrLoc = sockEcoute.addrLoc;

    return sd; 
}


/**
 *	\fn			socket_t connecterClt2Srv (char *adrIP, short port)
 *	\brief		Crétaion d'une socket d'appel et connexion au seveur dont
 *				l'adressage est fourni en paramètre
 *	\param		adrIP : adresse IP du serveur à connecter
 *	\param		port : port TCP du serveur à connecter
 *	\result		socket connectée au serveur fourni en paramètre
 */
socket_t connecterClt2Srv (char *adrIP, short port) {

    socket_t sock = creerSocket(SOCK_STREAM);

    socklen_t lenMyAddr = sizeof(sock.addrLoc);

    adr2struct(&(sock.addrDst), adrIP, port);

    CHECK(connect(sock.fd, (struct sockaddr *)&(sock.addrDst), sizeof(sock.addrDst)),"--connect()--");

    CHECK(getsockname(sock.fd, (struct sockaddr *)&(sock.addrLoc), &lenMyAddr),"--getsockname()--");


    return sock;
}