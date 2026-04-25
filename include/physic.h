#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>


// Partie physique du jeu 
// Structure de la balle 

typedef struct{
    // position
    float x, y; //position au sol de la balle
    float z; // position de la balle en hauteur

    // vélocité
    float vx, vy; // vélocité sur le sol 
    float vz; //vélocité en l'air

    float ral; // ralentissement de la balle

    bool inAir; // dans les airs
    bool inMovement; // en mouvement

} ball_t; 

// prototype des fonctions

void initBall(ball_t * ball, float startX, float startY, float startZ); // Initialisation de la balle et de sa position
void updateBallGround(ball_t * ball, float dt); // physique au sol
void updateBallAir(ball_t * ball, float dt); // physique dans les airs; 
void shoot(ball_t * ball); // tir de la ball
// collisions
void collisionWall(ball_t * ball); 
void collisionRampe(ball_t * ball); 
