#ifndef MAP_H
#define MAP_H

#include "raylib/raylib.h"


#define MAX_HEIGHTMAP_DIMENSION 50
#define Y_SCALE 0.5

#define MAX_MAPS 1


typedef struct {

    int width;
    int height;

    float heightmap[MAX_HEIGHTMAP_DIMENSION][MAX_HEIGHTMAP_DIMENSION];

    float hole_x;
    float hole_z;

    float start_x;
    float start_z;

    Mesh mesh;

} map_t;


typedef struct {
    float x;        // Position X world
    float y;        // Hauteur interpolée (Y world)
    float z;        // Position Z world
    float grad_x;   // Pente selon l'axe X
    float grad_z;   // Pente selon l'axe Z
    Vector3 normal; // Vecteur normal du sol à ce point précis
} ground_info_t;


void load_map(map_t * map, char * path);
Model get_map_model(map_t * map);
ground_info_t get_ground_info(map_t *map, float x, float z);
Mesh gen_map_borders(map_t *map, float baseHeight);

void render_map(Model water_model, Model map_model, Model border_model, map_t * map);

#endif  // MAP_H