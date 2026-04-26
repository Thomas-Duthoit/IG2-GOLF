#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "raylib/raymath.h"


#define printMap(fmt, ...) printf("\x1b[1;37;42mMAP\x1b[0m] " fmt, ##__VA_ARGS__)




void box_blur(map_t *map);
void gen_mesh_map(map_t * map);



void load_map(map_t * map, char * path) {
    
    printMap("Ouverture de %s\n", path);

    FILE *f_map = NULL;
    f_map = fopen(path, "r");
    
    if (f_map == NULL) {

        printMap("Erreur d'ouverture !\n");
        return;

    }


    char line[1000];

    fgets(line, sizeof(line), f_map);

    sscanf(line, "%d %d", &(map->width), &(map->height));

    printMap("Taille : W=%d, H=%d\n", map->width, map->height);

    for (int z = 0; z < map->height; z++) {
        fgets(line, sizeof(line), f_map);

        char *token = strtok(line, " ");

        for (int x = 0; x < map->width; x++) {

            if (token == NULL) {
                printMap("Erreur parsing ligne %d\n", z);
                break;
            }

            map->heightmap[z][x] = atoi(token);

            printMap("(x=%d;y=%d) -> heightmap=%f\n", x, z, map->heightmap[z][x]);

            token = strtok(NULL, " ");
        }
    }

    fgets(line, sizeof(line), f_map);
    sscanf(line, "%f %f", &(map->hole_x), &(map->hole_z));
    printMap("Drapeau en %f;%f\n", map->hole_x, map->hole_z);

    fgets(line, sizeof(line), f_map);
    sscanf(line, "%f %f", &(map->start_x), &(map->start_z));
    printMap("Départ en %f;%f\n", map->start_x, map->start_z);

    while (fgets(line, sizeof(line), f_map))
    {
        
    }




    fclose(f_map);    


    printMap("Parsing fini !\n");
    printMap("Traitement des données...\n");



    box_blur(map);
    gen_mesh_map(map);



    printMap("Traitement des données fini !\n");
}


Model get_map_model(map_t * map) {
    UploadMesh(&(map->mesh), false);
    return LoadModelFromMesh(map->mesh);
}




void gen_mesh_map(map_t * map) {
    int numQuads = (map->width - 1) * (map->height - 1);
    int triangleCount = numQuads * 2;
    int vertexCount = triangleCount * 3; // 3 sommets par triangle, sans partage

    map->mesh.vertexCount = vertexCount;
    map->mesh.triangleCount = triangleCount;

    map->mesh.vertices = malloc(vertexCount * 3 * sizeof(float));
    map->mesh.normals  = malloc(vertexCount * 3 * sizeof(float));
    // On n'a plus strictement besoin d'indices si on dessine dans l'ordre, 
    // mais Raylib aime bien en avoir ou on peut s'en passer en mettant indices à NULL.
    map->mesh.indices = NULL; 

    int vIndex = 0;

    for (int z = 0; z < map->height - 1; z++) {
        for (int x = 0; x < map->width - 1; x++) {
            
            // Les 4 coins d'un carré (quad)
            Vector3 v0 = { (float)x,     (float)map->heightmap[z][x] * Y_SCALE,     (float)z };
            Vector3 v1 = { (float)x + 1, (float)map->heightmap[z][x+1] * Y_SCALE,   (float)z };
            Vector3 v2 = { (float)x,     (float)map->heightmap[z+1][x] * Y_SCALE,   (float)z + 1 };
            Vector3 v3 = { (float)x + 1, (float)map->heightmap[z+1][x+1] * Y_SCALE, (float)z + 1 };

            // --- Triangle 1 (v0, v2, v1) ---
            // Calcul de la normale du triangle (une seule pour les 3 sommets)
            Vector3 n1 = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(v2, v0), Vector3Subtract(v1, v0)));
            
            float* verts = map->mesh.vertices;
            float* norms = map->mesh.normals;

            // Sommet 0
            verts[vIndex*3+0] = v0.x; verts[vIndex*3+1] = v0.y; verts[vIndex*3+2] = v0.z;
            norms[vIndex*3+0] = n1.x; norms[vIndex*3+1] = n1.y; norms[vIndex*3+2] = n1.z;
            vIndex++;
            // Sommet 2
            verts[vIndex*3+0] = v2.x; verts[vIndex*3+1] = v2.y; verts[vIndex*3+2] = v2.z;
            norms[vIndex*3+0] = n1.x; norms[vIndex*3+1] = n1.y; norms[vIndex*3+2] = n1.z;
            vIndex++;
            // Sommet 1
            verts[vIndex*3+0] = v1.x; verts[vIndex*3+1] = v1.y; verts[vIndex*3+2] = v1.z;
            norms[vIndex*3+0] = n1.x; norms[vIndex*3+1] = n1.y; norms[vIndex*3+2] = n1.z;
            vIndex++;

            // --- Triangle 2 (v1, v2, v3) ---
            Vector3 n2 = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(v2, v1), Vector3Subtract(v3, v1)));

            // Sommet 1
            verts[vIndex*3+0] = v1.x; verts[vIndex*3+1] = v1.y; verts[vIndex*3+2] = v1.z;
            norms[vIndex*3+0] = n2.x; norms[vIndex*3+1] = n2.y; norms[vIndex*3+2] = n2.z;
            vIndex++;
            // Sommet 2
            verts[vIndex*3+0] = v2.x; verts[vIndex*3+1] = v2.y; verts[vIndex*3+2] = v2.z;
            norms[vIndex*3+0] = n2.x; norms[vIndex*3+1] = n2.y; norms[vIndex*3+2] = n2.z;
            vIndex++;
            // Sommet 3
            verts[vIndex*3+0] = v3.x; verts[vIndex*3+1] = v3.y; verts[vIndex*3+2] = v3.z;
            norms[vIndex*3+0] = n2.x; norms[vIndex*3+1] = n2.y; norms[vIndex*3+2] = n2.z;
            vIndex++;
        }
    }
}



ground_info_t get_ground_info(map_t *map, float x, float z) {
    ground_info_t info = { 0 };
    info.x = x;
    info.z = z;

    // 4 coins entourant la position dans la heightmap
    int x0 = (int)floorf(x);
    int z0 = (int)floorf(z);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // clamping pour rester dans les limites de la map
    if (x0 < 0) x0 = 0; if (x0 >= map->width) x0 = map->width - 1;
    if (x1 < 0) x1 = 0; if (x1 >= map->width) x1 = map->width - 1;
    if (z0 < 0) z0 = 0; if (z0 >= map->height) z0 = map->height - 1;
    if (z1 < 0) z1 = 0; if (z1 >= map->height) z1 = map->height - 1;

    // 4 hauteurs en brut
    float h00 = (float)map->heightmap[z0][x0] * Y_SCALE;
    float h10 = (float)map->heightmap[z0][x1] * Y_SCALE;
    float h01 = (float)map->heightmap[z1][x0] * Y_SCALE;
    float h11 = (float)map->heightmap[z1][x1] * Y_SCALE;

    // 3interpolation normalisée entre 0 et 1 pour chaque axe (UV sur la cell)
    float u = x - (float)x0;
    float v = z - (float)z0;

    // interpolation bilineaire pour la hauteur
    // Formule : h(u,v) = h00(1-u)(1-v) + h10(u)(1-v) + h01(1-u)(v) + h11(uv)
    info.y = h00 * (1 - u) * (1 - v) +
             h10 * u * (1 - v) +
             h01 * (1 - u) * v +
             h11 * u * v;

    // gradients
    info.grad_x = (h10 - h00) * (1 - v) + (h11 - h01) * v;
    info.grad_z = (h01 - h00) * (1 - u) + (h11 - h10) * u;

    // calcul de la normale pour les rebonds
    Vector3 normal = { -info.grad_x, 1.0f, -info.grad_z };
    info.normal = Vector3Normalize(normal);

    return info;
}



void box_blur(map_t *map) {
    float temp[map->height][map->width];

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            float sum = 0.0f;
            int count = 0;

            // On regarde les voisins dans un carré de 3x3 autour du point (x, y)
            for (int ny = -1; ny <= 1; ny++) {
                for (int nx = -1; nx <= 1; nx++) {
                    int tx = x + nx;
                    int ty = y + ny;

                    // On vérifie si le voisin est bien dans la map
                    if (tx >= 0 && tx < map->width && ty >= 0 && ty < map->height) {
                        sum += map->heightmap[ty][tx];
                        count++;
                    }
                }
            }
            // On stocke la moyenne
            temp[y][x] = (float)(sum / count);
        }
    }

    // On recopie le résultat dans la map d'origine
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            map->heightmap[y][x] = temp[y][x];
        }
    }
}













void add_quad(Vector3 p1, Vector3 p2, float base, float* v, float* n, int* idx) {
    Vector3 p1_low = { p1.x, base, p1.z };
    Vector3 p2_low = { p2.x, base, p2.z };

    // --- Triangle 1 (p1, p2, p1_low) --- 
    Vector3 n1 = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(p2, p1), Vector3Subtract(p1_low, p1)));
    
    v[(*idx)*3+0]=p1.x;     v[(*idx)*3+1]=p1.y;     v[(*idx)*3+2]=p1.z; 
    n[(*idx)*3+0]=n1.x;     n[(*idx)*3+1]=n1.y;     n[(*idx)*3+2]=n1.z; 
    (*idx)++;
    
    v[(*idx)*3+0]=p2.x;     v[(*idx)*3+1]=p2.y;     v[(*idx)*3+2]=p2.z; 
    n[(*idx)*3+0]=n1.x;     n[(*idx)*3+1]=n1.y;     n[(*idx)*3+2]=n1.z; 
    (*idx)++;

    v[(*idx)*3+0]=p1_low.x; v[(*idx)*3+1]=p1_low.y; v[(*idx)*3+2]=p1_low.z; 
    n[(*idx)*3+0]=n1.x;     n[(*idx)*3+1]=n1.y;     n[(*idx)*3+2]=n1.z; 
    (*idx)++;

    // --- Triangle 2 (p2, p2_low, p1_low) ---
    Vector3 n2 = n1; 

    v[(*idx)*3+0]=p2.x;     v[(*idx)*3+1]=p2.y;     v[(*idx)*3+2]=p2.z; 
    n[(*idx)*3+0]=n2.x;     n[(*idx)*3+1]=n2.y;     n[(*idx)*3+2]=n2.z; 
    (*idx)++;

    v[(*idx)*3+0]=p2_low.x; v[(*idx)*3+1]=p2_low.y; v[(*idx)*3+2]=p2_low.z; 
    n[(*idx)*3+0]=n2.x;     n[(*idx)*3+1]=n2.y;     n[(*idx)*3+2]=n2.z; 
    (*idx)++;

    v[(*idx)*3+0]=p1_low.x; v[(*idx)*3+1]=p1_low.y; v[(*idx)*3+2]=p1_low.z; 
    n[(*idx)*3+0]=n2.x;     n[(*idx)*3+1]=n2.y;     n[(*idx)*3+2]=n2.z; 
    (*idx)++;
}




Mesh gen_map_borders(map_t *map, float baseHeight) {
    // Il y a (W-1 + H-1 + W-1 + H-1) segments sur le tour
    int edgeSegments = (map->width - 1) * 2 + (map->height - 1) * 2;
    int triangleCount = edgeSegments * 2; // 2 triangles par segment (un quad)
    int vertexCount = triangleCount * 3;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = malloc(vertexCount * 3 * sizeof(float));
    mesh.normals = malloc(vertexCount * 3 * sizeof(float));

    int vIdx = 0;

    
    // Bord Z = 0 (Haut)
    for (int x = 0; x < map->width - 1; x++) {
        Vector3 p1 = { (float)x, (float)map->heightmap[0][x] * Y_SCALE, 0 };
        Vector3 p2 = { (float)x + 1, (float)map->heightmap[0][x+1] * Y_SCALE, 0 };
        add_quad(p1, p2, baseHeight, mesh.vertices, mesh.normals, &vIdx);
    }
    // Bord Z = H-1 (Bas)
    for (int x = 0; x < map->width - 1; x++) {
        Vector3 p1 = { (float)x+1, (float)map->heightmap[map->height-1][x+1] * Y_SCALE, (float)map->height-1 };
        Vector3 p2 = { (float)x, (float)map->heightmap[map->height-1][x] * Y_SCALE, (float)map->height-1 };
        add_quad(p1, p2, baseHeight, mesh.vertices, mesh.normals, &vIdx);
    }
    // Bord X = 0 (Gauche)
    for (int z = 0; z < map->height - 1; z++) {
        Vector3 p1 = { 0, (float)map->heightmap[z+1][0] * Y_SCALE, (float)z+1 };
        Vector3 p2 = { 0, (float)map->heightmap[z][0] * Y_SCALE, (float)z };
        add_quad(p1, p2, baseHeight, mesh.vertices, mesh.normals, &vIdx);
    }
    // Bord X = W-1 (Droite)
    for (int z = 0; z < map->height - 1; z++) {
        Vector3 p1 = { (float)map->width-1, (float)map->heightmap[z][map->width-1] * Y_SCALE, (float)z };
        Vector3 p2 = { (float)map->width-1, (float)map->heightmap[z+1][map->width-1] * Y_SCALE, (float)z+1 };
        add_quad(p1, p2, baseHeight, mesh.vertices, mesh.normals, &vIdx);
    }

    UploadMesh(&mesh, false);
    return mesh;
}






void render_map(Model water_model, Model map_model, Model border_model, map_t * map) {
    float waterHeight = -2.0f;

    DrawModel(map_model, (Vector3){0,0,0}, 1.0f, WHITE);

    ground_info_t hole_ground_info = get_ground_info(map, map->hole_x, map->hole_z);
    float h_y = hole_ground_info.y;

    DrawCylinderEx(
        (Vector3){ map->hole_x, h_y, map->hole_z }, 
        (Vector3){ map->hole_x, h_y + 3.0f, map->hole_z }, 
        0.05f, 0.05f, 8, WHITE
    );
    
    Vector3 tip = { map->hole_x, h_y + 3.0f, map->hole_z};
    Vector3 base = { map->hole_x, h_y + 3.0f - 0.8f, map->hole_z };
    Vector3 edge = { map->hole_x + 1.0f, h_y + 3.0f - 0.4f, map->hole_z +1.0f};

    // CW et CCW pour être affiché de partout
    DrawTriangle3D(tip, edge, base, RED);
    DrawTriangle3D(base, edge, tip, RED);

    DrawCylinderEx(
        (Vector3){ map->hole_x, h_y + 0.02f, map->hole_z },
        (Vector3){ map->hole_x, h_y + 0.025f,map->hole_z },
        0.3f, 0.3f, 16, BLACK
    );


    DrawModel(border_model, (Vector3){0,0,0}, 1.0f, WHITE);

    Vector3 waterPos = { map->width/2.0f, waterHeight, map->height/2.0f };
    DrawModel(water_model, waterPos, 1.0f, WHITE);
}