#include "physic.h"

// Prototype fonction interne
void checkArret(ground_info_t ground_info, ball_t * ball);  
void updateValue(ground_info_t ground_info, Vector3 * pos_th, ball_t * ball, float dt);  



void init_pos_ball(ball_t * ball, float startX, float startZ, float startY){
    // Mise en place des positions initiales à l'horizontal
    ball->pos.x = startX; 
    ball->pos.z = startZ; 

    // position initial à la vertical
    ball->pos.y = startY; 

    // Mise en place de la vélocité de la balle initiale
    ball->vel.x = 0; 
    ball->vel.y = 0; 
    ball->vel.z = 0; 

    // Ralentissement
    ball->ral = 0.999f;

    // Etat dans position
    ball->inAir = false; 
    ball->inMovement = false; 

    ball->inHole = false; 

}





void update_ball_mov(ball_t * ball, double dt, map_t * map){
    Vector3 pos_th; // position théorique calculée
    ground_info_t ground_info; // information du sol

    if (ball->inMovement == false) return; 

    ball->vel.y = ball->vel.y - (G * dt);     // Calcul de la gravité de la balle

    pos_th = Vector3Add(ball->pos, Vector3Scale(ball->vel, dt)); 

    if (
        ball->pos.x >= 0 
        && ball->pos.y >= 0
        && ball->pos.x <= map->width
        && ball->pos.y <= map->height
    ) {
        ground_info = get_ground_info(map, ball->pos.x, ball->pos.z); 

        updateValue(ground_info, &(pos_th), ball, dt); 
    }

    ball->pos.x = pos_th.x; 
    ball->pos.y = pos_th.y; 
    ball->pos.z = pos_th.z; 

    checkArret(ground_info, ball);    

    float waterHeight = -2.0f;
    if (ball->pos.y < (waterHeight-3.0)) {
        ground_info_t start_ground_info = get_ground_info(map, map->start_x, map->start_z);
        ball->vel = (Vector3) {0.0, 0.0, 0.0};
        init_pos_ball(ball, map->start_x, map->start_z, start_ground_info.y);
    }
}



void isInHole(ball_t * ball, map_t * map){
    ground_info_t ground_info; 

    ground_info = get_ground_info(map, ball->pos.x, ball->pos.z); 

    if((ball->pos.x - map->hole_x < DIS_HOLE_OK) && (ball->pos.z - map->hole_z < DIS_HOLE_OK) && (ball->pos.y - ground_info.y < DIS_HOLE_OK)
                                                    && (ball->vel.x < VEL_HOLE_OK) && (ball->vel.z < VEL_HOLE_OK) && (ball->vel.y < VEL_HOLE_OK) ){
        ball->inHole = true; 
        return; 
    }else{
        ball->inHole = false; 
        return; 
    }
}





/*-------------------------INTERNE------------------------*/
void checkArret(ground_info_t ground_info, ball_t * ball){

    if( (fabsf(ground_info.grad_x) < EPSILON_GRAD) && (fabsf(ground_info.grad_z) < EPSILON_GRAD) && (fabsf(ball->vel.x) <  EPSILON_VEL) && (fabsf(ball->vel.y) <  EPSILON_VEL) && (fabsf(ball->vel.z) <  EPSILON_VEL)){
        ball->inMovement = false; 
        ball->vel.x = 0; 
        ball->vel.y = 0; 
        ball->vel.z = 0; 
    }


}


void updateValue(ground_info_t ground_info, Vector3 * pos_th, ball_t * ball, float dt){

    if(pos_th->y < ground_info.y + BALL_RADIUS){

        // Mise à la surface
        pos_th->y = ground_info.y + BALL_RADIUS; 

        // Pente
        ball->vel.x = ball->vel.x - (ground_info.grad_x * KGRAD * dt); 
        ball->vel.z = ball->vel.z - (ground_info.grad_z * KGRAD * dt); 





        // Détection d'un creux — les gradients pointent vers la balle
        // (convergence des pentes = fond de trou)
        float grad_mag = sqrtf(ground_info.grad_x * ground_info.grad_x 
                               + ground_info.grad_z * ground_info.grad_z);

        if(grad_mag > EPSILON_GRAD) {
            // Direction vers laquelle la pente pousse la balle
            float dir_x = -ground_info.grad_x / grad_mag;
            float dir_z = -ground_info.grad_z / grad_mag;

            // Vitesse dans la direction de la pente
            float vel_along = ball->vel.x * dir_x + ball->vel.z * dir_z;

            // Si la balle va dans le sens opposé à la pente → elle remonte
            // → force de rappel centripète (balancier)
            if(vel_along < 0) {
                float force_pendul = KPENDUL * grad_mag;
                ball->vel.x += dir_x * force_pendul * dt;
                ball->vel.z += dir_z * force_pendul * dt;
            }
        }






        // friction au sol de la balle
        ball->vel = Vector3Scale(ball->vel, ball->ral); 

        // projection sur normale
        ball->vel = Vector3Subtract(ball->vel, Vector3Scale(ground_info.normal, Vector3DotProduct(ball->vel, ground_info.normal))); 


        if (ball->vel.y < KVREBOND){        // Rebond
            ball->vel.y = ball->vel.y * (-1.0f/3.0f); 
        }

    }


}