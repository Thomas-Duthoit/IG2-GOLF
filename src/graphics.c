#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "graphics.h"
#include "map.h"



Model map_models[MAX_MAPS];
Mesh map_border_meshes[MAX_MAPS];
Model map_border_models[MAX_MAPS];

Model water_model;

Model ball_model;

Shader shader;


int lightDirLoc;
int lightColLoc;
int ambientLoc;
int viewPosLoc;




Camera3D camera = {0};




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
    Mesh ball_mesh = GenMeshSphere(0.3f, 16, 16); 
    ball_model = LoadModelFromMesh(ball_mesh); 
    ball_model.materials[0].shader = shader; 
}


void render_current_map(map_t *maps, int map_idx) {

    SetShaderValue(shader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);  
            
    render_map(water_model, map_models[map_idx], map_border_models[map_idx], &maps[map_idx]);
}

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
        ball->x, 
        ball->y + 0.3f, 
        ball->z
    }; 

    ball_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color; 

    DrawModel(ball_model, position, 1.0f, WHITE); 


}

void render_ball_name(ball_t * ball, name_t player_name){

    Vector3 text_pos_3d = {ball->x, ball->y + 0.8f, ball->z}; 

    Vector2 text_pos_2d = GetWorldToScreen(text_pos_3d, camera); 

    int text_width = MeasureText(player_name, 10); 
    DrawText(player_name, (int)text_pos_2d.x - text_width / 2, (int) text_pos_2d.y, 10, WHITE);

}