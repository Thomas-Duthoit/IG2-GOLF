#include <raylib/raylib.h>
#include "users.h"
#include "update.h"
#include "map.h"
#include "dial.h"
#include "graphics.h"

extern short PORT_SRV_APP;
extern char IP_SERVICE[100];

extern users_t hotes;
extern users_t clients_app; 
extern users_t clients; 
extern user_t hote_serv_app;  

extern game_state_t game_state; 

extern double startCountdownTime;
extern double endScreenTime;
extern double nextCountdownTime; 

extern name_t pseudo;
extern name_t pseudo_next_player; 

extern bool next_player; 

extern map_t maps[MAX_MAPS];
extern int current_map;

extern cam_mode_t camera_mode;
extern bool aiming;  // en train de viser pour tirer
extern float shoot_puissance;
extern Vector3 shoot_direction;
extern Vector2 mouse_delta;

extern bool can_shoot;  // on peut tirer ou non


extern bool set_ball_pos_envoye;

extern ball_t balls[MAX_USERS];
extern int my_ball_index;  // -1 = pas trouvé, sinon index

extern int scores[NB_JOUEURS_MAX][NB_MANCHE]; // Podium



#pragma region LIST

void renderLIST(){
    // partie affichage
    BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawText("ETAT :", 10, 10, 20, BLACK);

        // bouton host
        DrawRectangle(10, 30, 30, 30, GRAY);
        DrawText("H", 13, 33, 20, BLACK);

        // bouton online
        DrawRectangle(50, 30, 30, 30, GRAY);
        DrawText("O", 53, 33, 20, BLACK);

        // bouton full
        DrawRectangle(90, 30, 30, 30, GRAY);
        DrawText("F", 93, 33, 20, BLACK);

        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

            
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {  // clic gauche


            // bouton host
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 30, 30, 30})) {
                DrawRectangle(10, 30, 30, 30, GREEN);
                DrawText("H", 13, 33, 20, DARKGREEN);
            }

            // bouton online
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){50, 30, 30, 30})) {
                DrawRectangle(50, 30, 30, 30, GREEN);
                DrawText("O", 53, 33, 20, DARKGREEN);
            }

            // bouton full
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){90, 30, 30, 30})) {
                DrawRectangle(90, 30, 30, 30, GREEN);
                DrawText("F", 93, 33, 20, DARKGREEN);
            }
        }

        // affichage @IP + port
        DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 10, 70, 20, BLACK);
        DrawText("Liste des hotes : ", 10, 90, 20, BLACK);

        for (int i=0; i<hotes.nbUsers; i++) {
            DrawRectangle(10, 110+(20*i), 20, 20, GRAY);
            DrawText(">", 10, 110+(20*i), 20, BLACK);
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){10, 110+(20*i), 20, 20})) {
                DrawRectangle(10, 110+(20*i), 20, 20, GREEN);
                DrawText(">", 10, 110+(20*i), 20, DARKGREEN);
            }
            DrawText(TextFormat("> %s  - %s : %hu", hotes.tab[i].name, hotes.tab[i].adrIP, hotes.tab[i].port_srv_app), 40, 110+(20*i), 20, BLACK);
        }



        DrawFPS(10, 450-20);
    
    EndDrawing();
}

#pragma endregion



#pragma region LOBBY

void renderLOBBY(){
    // partie affichage
    BeginDrawing();

        ClearBackground(RAYWHITE);

        // bouton quitter
        DrawRectangle(760, 30, 30, 30, GRAY);
        DrawText("X", 770, 35, 20, BLACK);

        // bouton start
        if(clients_app.nbUsers > 1){
            DrawRectangle(700, 430, 100, 20, GRAY);
            DrawText("START", 720, 430, 20, BLACK);
        }


        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            DrawRectangle(760, 30, 30, 30, RED);
            DrawText("X", 770, 35, 20, DARKGRAY);
        }

        if(clients_app.nbUsers > 1){
            // bouton start
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){700, 430, 100, 20})) {
                DrawRectangle(700, 430, 100, 20, GREEN);
                DrawText("START", 720, 430, 20, DARKGREEN);
            }
        }


        // affichage @IP + port
        DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 10, 10, 20, BLACK);
        
        
        for (int i=0; i<clients_app.nbUsers; i++) {
            DrawText(TextFormat("> %s ", clients_app.tab[i].name), 40, 110+(20*i), 20, BLACK);
        }


        DrawFPS(10, 450-20);

    EndDrawing();
}

#pragma endregion



#pragma region LOBBY Client

void renderLOBBYClt(){
    // partie affichage
    BeginDrawing();

        ClearBackground(RAYWHITE);

        // bouton quitter
        DrawRectangle(760, 30, 30, 30, GRAY);
        DrawText("X", 770, 35, 20, BLACK);


        // récupération de la position de la souris
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // bouton quitter
        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
            DrawRectangle(760, 30, 30, 30, RED);
            DrawText("X", 770, 35, 20, DARKGRAY);
        }

        // TODO : Voir pour mettre le pseudo dans une autre couleur
        DrawText(TextFormat("Bienvenue dans le serveur de [%s] !!", hote_serv_app.name), 10, 10, 20, BLACK);
        // affichage @IP + port
        //DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 20, 20, 20, BLACK);
        
        
        for (int i=0; i<clients.nbUsers; i++) {
            DrawText(TextFormat("> %s ", clients.tab[i].name), 40, 110+(20*i), 20, BLACK);
        }


        DrawFPS(10, 450-20);

    EndDrawing();
}

#pragma endregion




#pragma region START

void renderSTART(){
    // partie affichage
    BeginDrawing();
        ClearBackground(RAYWHITE);

        double elapsed = GetTime() - startCountdownTime;
        int remaining = 3 - (int)elapsed;

        if (remaining > 0) {
            DrawText("Debut de la partie dans :", 220, 150, 30, BLACK);
            DrawText(TextFormat("%d", remaining), 380, 220, 60, RED);
        }
        else {
            game_state = GAME;
            DisableCursor();
            startCountdownTime = 0; 
        }

        DrawFPS(10, 450-20);

    EndDrawing();

}

#pragma endregion


#pragma region GAME

void renderGAME(){
    // partie affichage

    int nbUsers; 

    BeginDrawing();

        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

            render_current_map(maps, current_map);

            nbUsers = estHote() ? clients_app.nbUsers : clients.nbUsers; 
            
            for (int i = 0; i < nbUsers; i++){
                if(!(balls[i].inHole)){
                   render_ball(&(balls[i]), i); 
                }
            }

            if (aiming && my_ball_index != -1) {
                Vector3 start = balls[my_ball_index].pos;
                Vector3 end = Vector3Add(start, Vector3Scale(shoot_direction, shoot_puissance * 0.5f));
                
                DrawLine3D(start, end, RED);
                DrawSphere(end, 0.1f, RED);
            }

        EndMode3D();


        if(estHote()){
            for (int i = 0; i < clients_app.nbUsers; i++){
                if(!(balls[i].inHole))
                    render_ball_name(&(balls[i]), clients_app.tab[i].name);
            }
        }
        else{
            for (int i = 0; i < clients.nbUsers; i++){
                if(!(balls[i].inHole))
                    render_ball_name(&(balls[i]), clients.tab[i].name); 
            }
        } 


        if (estHote()){
            // bouton quitter
            DrawRectangle(760, 30, 30, 30, GRAY);
            DrawText("X", 770, 35, 20, BLACK);

            DrawRectangle(20, 20, 20, 20, GRAY);
            DrawText(">", 30, 25, 20, BLACK); 

            DrawRectangle(20, 60, 20, 20, GRAY); 
            DrawText("N", 30, 65, 20, BLACK);

            // récupération de la position de la souris
            int mouse_x = GetMouseX();
            int mouse_y = GetMouseY();

            // bouton fin de partie
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){760, 30, 30, 30})) {
                DrawRectangle(760, 30, 30, 30, RED);
                DrawText("X", 770, 35, 20, DARKGRAY);
            }

            // bouton next player
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){20, 20, 20, 20})) {
                DrawRectangle(20, 20, 20, 20, GREEN);
                DrawText(">", 30, 25, 20, DARKGRAY);
            }

            // bouton next round
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, (Rectangle){20, 60, 20, 20})) {
                DrawRectangle(20, 60, 20, 20, GREEN);
                DrawText("N", 30, 65, 20, DARKGRAY);
            }
        }

        //DrawText("La partie vient de commencer !!", 220, 150, 30, BLACK);

        if(next_player == true){
            if(strcmp(pseudo, pseudo_next_player) == 0){
                DrawText(TextFormat("C'est mon tour de jouer !"), 300, 10, 20, BLACK); 
            }
            else{
                DrawText(TextFormat("Joueur qui doit jouer : %s", pseudo_next_player), 300, 10, 20, BLACK); 
            }
        }


        DrawFPS(10, 450-20);

    EndDrawing();

}

#pragma endregion





#pragma region END

void renderEND(){

    // partie affichage
    BeginDrawing();

        ClearBackground(RAYWHITE);


        DrawText("La partie est terminée !!", 220, 150, 30, BLACK);


        DrawFPS(10, 450-20);

    EndDrawing();

}

#pragma endregion




#pragma region NEXT

void renderNEXT(){
    // partie affichage
    BeginDrawing();
        ClearBackground(RAYWHITE);

        double elapsed = GetTime() - nextCountdownTime;
        int remaining = 3 - (int)elapsed;

        if (remaining > 0) {
            DrawText("Prochaine manche dans :", 220, 150, 30, BLACK);
            DrawText(TextFormat("%d", remaining), 380, 220, 60, RED);
        }
        else {
            game_state = GAME;
            DisableCursor();
            nextCountdownTime = 0; 
        }

        DrawFPS(10, 450-20);

    EndDrawing();
}

#pragma endregion



#pragma region PODIUM

void renderPODIUM(){

    int nbUsers = estHote() ? clients_app.nbUsers : clients.nbUsers; 

    users_t *u_list;
    if (estHote()) {
        u_list = &clients_app;
    } else {
        u_list = &clients;
    }

    BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Résultat :", 10, 10, 20, BLACK);


        // Affichage des pseudos
        DrawText("Joueurs", 100, 50, 20, BLACK); 
        for(int i = 0; i < nbUsers; i++){
            DrawText(TextFormat("%s", u_list->tab[i].name), 100, 50 + (i+1)*50, 20, BLACK); 
        }

        // Affichage score manche 1
        DrawText("1", 200, 50, 20, BLACK); 
        for(int i = 0; i < nbUsers; i++){
            DrawText(TextFormat("%d", scores[i][0]), 200, 50 + (i+1)*50, 20, BLACK); 
        }

        // Affichage score manche 2
        DrawText("2", 300, 50, 20, BLACK); 
        for(int i = 0; i < nbUsers; i++){
            DrawText(TextFormat("%d", scores[i][1]), 300, 50 + (i+1)*50, 20, BLACK); 
        }

        // Affichage score manche 3
        DrawText("3", 400, 50, 20, BLACK); 
        for(int i = 0; i < nbUsers; i++){
            DrawText(TextFormat("%d", scores[i][2]), 400, 50 + (i+1)*50, 20, BLACK); 
        }

        // Affichage total
        DrawText("Total", 500, 50, 20, BLACK); 
        for(int i = 0; i < nbUsers; i++){
            DrawText(TextFormat("%d", (scores[i][0] + scores[i][1] + scores[i][2])), 500, 50 + (i+1)*50, 20, BLACK); 
        }



        DrawFPS(10, 450-20);

    EndDrawing();
}

#pragma endregion