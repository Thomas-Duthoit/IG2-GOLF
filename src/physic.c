#include "physic.h"

// Prototype fonction interne
void upt_ball_pos(ball_t * ball, float dt); 
void upt_ball_vel(ball_t * ball); 
void upt_ball_pos_air(ball_t * ball); 
void upt_ball_vel_air(ball_t * ball); 


void initBall(ball_t * ball, float startX, float startY, float startZ){
    // Mise en place des positions initiales
    ball->x = startX; 
    ball->y = startY; 
    ball->z = startZ; 

    // Mise en place de la vélocité de la balle initiale
    ball->vx = 0; 
    ball->vy = 0; 
    ball->vz = 0; 

    // Ralentissement
    ball->ral = 0.3;

    // Etat dans position
    ball->inAir = false; 
    ball->inMovement = false; 

}


void updateBallGround(ball_t * ball, float dt){
    upt_ball_pos(ball, dt); 
}





/*-------------------------INTERNE------------------------*/

void upt_ball_pos(ball_t * ball, float dt){
    ball->x += ball->vx * dt; 
    ball->y += ball->vy * dt; 
}