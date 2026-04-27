#ifndef PHYSIC_H
#define PHYSIC_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "map.h"


#define BALL_RADIUS 0.1     // Rayon de la balle
#define G 9.8               // constante de gravité
#define KGRAD 1             // constante accélération pente
#define KVREBOND 0.2        // velocité minimale en y pour un revond (< 0)
#define EPSILON_GRAD 0.1
#define EPSILON_VEL 0.01f
#define KPENDUL 15.0f       // force du balancier
#define RAYON_TROU 1.5f     // rayon de détection du trou

#define DIS_HOLE_OK 0.3f    // distance horizontal OK avec le trou
#define VEL_HOLE_OK 0.3f    // vélocité OK pour entrer dans le trou


// Partie physique du jeu 
// Structure de la balle 

typedef struct{
    // position
    Vector3 pos; // position de la balle

    // vélocité
    Vector3 vel; // velocité de la balle

    float ral; // ralentissement de la balle

    bool inAir; // dans les airs
    bool inMovement; // en mouvement

    bool inHole; 

} ball_t; 


// prototype des fonctions

void init_pos_ball(ball_t * ball, float startX, float startZ, float startY); // Initialisation de la balle et de sa position
void update_ball_mov(ball_t * ball, double dt, map_t * map); // update du mouvement de la balle


#endif // PHYSIC_H
