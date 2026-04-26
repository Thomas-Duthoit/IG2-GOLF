#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "map.h"

extern Camera3D camera;





void init_graphics(map_t *maps);
void render_current_map(map_t *maps, int map_idx);





#endif  // GRAPHICS_H