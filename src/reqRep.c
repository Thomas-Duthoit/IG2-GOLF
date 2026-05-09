/**
 * \file reqRep.c
 * \brief Conversion entre requêtes/réponses et chaînes de caractères pour la communication réseau
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#include <stdio.h>

#include "reqRep.h"

/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
/**
 * \fn void req2str(requete_t * req, char * str) 
 * \brief Sérialise une requête en chaîne de caractères pour l'envoi réseau
 * \param req Pointeur vers la requête source
 * \param str Chaîne de destination (doit être suffisamment grande)
 */
void req2str(requete_t * req, char * str) {
  // attention : il faut utiliser sNprintf pour que ça soit sécur...
  sprintf(str, REQ_IN_OUT, req->idReq, req->verbReq, req->optReq);

}

/**
 * \fn void str2req(char * str, requete_t * req)
 * \brief Désérialise une chaîne de caractères reçue du réseau en structure requête
 * \param str Chaîne source
 * \param req Pointeur vers la requête destination
 */
void str2req(char * str, requete_t * req){
  sscanf(str, REQ_OUT_IN, &req->idReq, req->verbReq, req->optReq);
}


/**
 * \fn void rep2str(requete_t * rep, char * str)
 * \brief Sérialise une réponse en chaîne de caractères pour l'envoi réseau
 * \param rep Pointeur vers la réponse source
 * \param str Chaîne de destination (doit être suffisamment grande)
 */
void rep2str(requete_t * rep, char * str) {
  // attention : il faut utiliser sNprintf pour que ça soit sécur...
  sprintf(str, REP_IN_OUT, rep->idReq, rep->verbReq, rep->optReq);

}

/**
 * \fn void str2rep(char * str, requete_t * rep)
 * \brief Désérialise une chaîne de caractères reçue du réseau en structure réponse
 * \param str Chaîne source
 * \param rep Pointeur vers la réponse destination
 */
void str2rep(char * str, requete_t * rep){
  sscanf(str, REP_OUT_IN, &rep->idReq, rep->verbReq, rep->optReq);
}

