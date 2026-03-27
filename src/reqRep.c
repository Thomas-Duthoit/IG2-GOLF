#include <stdio.h>

#include "reqRep.h"


void req2str(requete_t * req, char * str) {
  // attention : il faut utiliser sNprintf pour que ça soit sécur...
  sprintf(str, REQ_IN_OUT, req->idReq, req->verbReq, req->optReq);

}

void str2req(char * str, requete_t * req){
  sscanf(str, REQ_OUT_IN, &req->idReq, req->verbReq, req->optReq);
}


void rep2str(requete_t * rep, char * str) {
  // attention : il faut utiliser sNprintf pour que ça soit sécur...
  sprintf(str, REP_IN_OUT, rep->idReq, rep->verbReq, rep->optReq);

}

void str2rep(char * str, requete_t * rep){
  sscanf(str, REP_OUT_IN, &rep->idReq, rep->verbReq, rep->optReq);
}

