#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "map.h"
#include "physic.h"
#include "users.h"

extern Camera3D camera;




void init_graphics(map_t *maps);
void render_current_map(map_t *maps, int map_idx);
void render_ball(ball_t * ball, int player_index); 
void render_ball_name(ball_t * ball, name_t player_name); 





#endif  // GRAPHICS_H