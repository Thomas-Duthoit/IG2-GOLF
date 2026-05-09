/**
 * \file graphics.c
 * \brief Gestion des graphismes et rendu 3D avec Raylib
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "graphics.h"
#include "map.h"

/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    G L O B A L E S
 */

 /**
 * \var map_models
 * \brief Tableau des modèles 3D des cartes (un modèle par carte)
 */
Model map_models[MAX_MAPS];
/**
 * \var map_border_meshes
 * \brief Tableau des meshes des bordures de cartes (un mesh par carte)
 */
Mesh map_border_meshes[MAX_MAPS];
/**
 * \var map_border_models
 * \brief Tableau des modèles 3D des bordures de cartes (un modèle par carte)
 */
Model map_border_models[MAX_MAPS];

/**
 * \var water_model
 * \brief Modèle 3D du plan d'eau utilisé comme fond de scène
 */
Model water_model;
/**
 * \var ball_model
 * \brief Modèle 3D de la balle de golf (sphère partagée entre tous les joueurs, colorée dynamiquement)
 */
Model ball_model;

/**
 * \var shader
 * \brief Shader principal utilisé pour le rendu de tous les objets de la scène (éclairage directionnel)
 */
Shader shader;

/**
 * \var lightDirLoc
 * \brief Emplacement (location) de l'uniforme de direction de la lumière dans le shader
 */
int lightDirLoc;
/**
 * \var lightColLoc
 * \brief Emplacement (location) de l'uniforme de couleur de la lumière dans le shader
 */
int lightColLoc;
/**
 * \var ambientLoc
 * \brief Emplacement (location) de l'uniforme de lumière ambiante dans le shader
 */
int ambientLoc;
/**
 * \var viewPosLoc
 * \brief Emplacement (location) de l'uniforme de position de la caméra dans le shader (utilisé pour les reflets)
 */
int viewPosLoc;

/**
 * \var camera
 * \brief Caméra 3D principale utilisée pour le rendu de la scène de jeu
 */
Camera3D camera = {0};


/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

/**
 * \fn void init_graphics(map_t *maps)
 * \brief Initialise les graphismes, charge les shaders et génère les modèles 3D
 * \param maps Tableau des cartes du jeu
 */
void init_graphics(map_t *maps) {

    shader = LoadShader("shaders/vertex.glsl", "shaders/fragment.glsl");


    for (int i=0; i<MAX_MAPS; i++) {
        map_models[i] = get_map_model(&maps[i]);
        map_border_meshes[i] = gen_map_borders(&maps[i], -5.0f);
        map_border_models[i] = LoadModelFromMesh(map_border_meshes[i]);

        map_models[i].materials[0].shader = shader;
        map_models[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = (Color) {95, 138, 74, 255};

        map_border_models[i].materials[0].shader = shader;
        map_border_models[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BROWN; 
    }

    lightDirLoc = GetShaderLocation(shader, "lightDir");
    lightColLoc = GetShaderLocation(shader, "lightColor");
    ambientLoc  = GetShaderLocation(shader, "ambient");
    viewPosLoc = GetShaderLocation(shader, "viewPos");

    Vector3 sunDir = Vector3Normalize((Vector3){ -0.5f, -1.0f, -0.2f }); // Direction du soleil
    Vector4 sunCol = (Vector4){ 1.0f, 0.98f, 0.9f, 1.0f };              // Jaune chaud
    Vector4 ambCol = (Vector4){ 0.3f, 0.3f, 0.4f, 1.0f };              // Ombres bleutées

    SetShaderValue(shader, lightDirLoc, &sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, lightColLoc, &sunCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, ambientLoc,  &ambCol, SHADER_UNIFORM_VEC4);

    Mesh water_mesh = GenMeshPlane(1000, 1000, 10, 10); 
    water_model = LoadModelFromMesh(water_mesh);

    water_model.materials[0].shader = shader;
    water_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 0, 120, 210, 180 };


    camera.position = (Vector3){ -10, 20.0f, -10 };
    camera.target   = (Vector3){ 0, 0.0f, 0 };
    camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy     = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;


    // Balle 
    Mesh ball_mesh = GenMeshSphere(1.0f, 16, 16); 
    ball_model = LoadModelFromMesh(ball_mesh); 
    ball_model.materials[0].shader = shader; 
}


/**
 * \fn void render_current_map(map_t *maps, int map_idx)
 * \brief Rend la carte courante ainsi que ses bordures et le plan d'eau
 * \param maps    Tableau des cartes du jeu
 * \param map_idx Index de la carte à afficher
 */
void render_current_map(map_t *maps, int map_idx) {

    SetShaderValue(shader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);  
            
    render_map(water_model, map_models[map_idx], map_border_models[map_idx], &maps[map_idx]);
}

/**
 * \fn void render_ball(ball_t * ball, int player_index)
 * \brief Rend la balle d'un joueur à sa position courante avec sa couleur associée
 * \param ball         Pointeur vers la structure de la balle à afficher
 * \param player_index Index du joueur (détermine la couleur de la balle)
 */
void render_ball(ball_t * ball, int player_index){

    static const Color BALL_COLORS[] = {
        RED, 
        BLUE, 
        GREEN, 
        ORANGE, 
        PURPLE, 
        PINK 
    };

    Color color = BALL_COLORS[player_index % 6]; 

    Vector3 position = {
        ball->pos.x, 
        ball->pos.y + BALL_RADIUS, 
        ball->pos.z
    }; 

    ball_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color; 

    DrawModel(ball_model, position, BALL_RADIUS, WHITE); 


}

/**
 * \fn void render_ball_name(ball_t * ball, name_t player_name)
 * \brief Rend le nom du joueur en 2D au-dessus de sa balle dans la scène 3D
 * \param ball        Pointeur vers la structure de la balle
 * \param player_name Nom du joueur à afficher
 */
void render_ball_name(ball_t * ball, name_t player_name){

    Vector3 text_pos_3d = {ball->pos.x, ball->pos.y + 0.2f + BALL_RADIUS, ball->pos.z}; 

    Vector2 text_pos_2d = GetWorldToScreen(text_pos_3d, camera); 

    int text_width = MeasureText(player_name, 10); 
    DrawText(player_name, (int)text_pos_2d.x - text_width / 2, (int) text_pos_2d.y, 10, WHITE);

}