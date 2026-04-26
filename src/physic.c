#include "physic.h"

// Prototype fonction interne
void upt_ball_pos(ball_t * ball, float dt); 
void upt_ball_vel(ball_t * ball); 
void upt_ball_pos_air(ball_t * ball); 
void upt_ball_vel_air(ball_t * ball); 



void init_pos_ball(ball_t * ball, float startX, float startZ, float startY){
    // Mise en place des positions initiales à l'horizontal
    ball->x = startX; 
    ball->z = startZ; 

    // position initial à la vertical
    ball->y = startY; 

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