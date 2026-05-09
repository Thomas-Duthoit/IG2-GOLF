/**
 * \file render.c
 * \brief Gestion du rendu des différents états du jeu (LIST, LOBBY, GAME, PODIUM, etc.)
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#include <raylib/raylib.h>
#include "users.h"
#include "update.h"
#include "map.h"
#include "dial.h"
#include "graphics.h"

/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    E X T E R N E S
 *  (descriptions déjà donné dans client.c)
 */

// Configuration réseau
extern short PORT_SRV_APP;
extern char IP_SERVICE[100];

// Données utilisateurs
extern users_t hotes;
extern users_t clients_app; 
extern users_t clients; 
extern user_t hote_serv_app;  

// État du jeu
extern game_state_t game_state; 

// Timers d'état
extern double startCountdownTime;
extern double endScreenTime;
extern double nextCountdownTime; 

// Identité du joueur
extern name_t pseudo;
extern name_t pseudo_next_player; 

// Drapeaux de tour de jeu
extern bool next_player; 

// Cartes
extern map_t maps[MAX_MAPS];
extern int current_map;

// Caméra et tir
extern cam_mode_t camera_mode;
extern bool aiming;  // en train de viser pour tirer
extern float shoot_puissance;
extern Vector3 shoot_direction;
extern Vector2 mouse_delta;
extern bool can_shoot;  // on peut tirer ou non
extern bool set_ball_pos_envoye;

// Balles
extern ball_t balls[MAX_USERS];
extern int my_ball_index;  // -1 = pas trouvé, sinon index

// Podium
extern int scores[NB_JOUEURS_MAX][NB_MANCHE]; // Podium

/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

#pragma region LIST

/**
 * \fn void renderList()
 * \brief Rendue de l'écran de liste des hôtes disponibles
 */
void renderLIST(){
    BeginDrawing();

        ClearBackground(SKYBLUE);

        Rectangle hostButton = {300, 100, 200, 60};
        DrawRectangleRec(hostButton, GRAY);
        DrawText("HÔTE", hostButton.x + 75, hostButton.y + 20, 20, BLACK);

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, hostButton)) {
            DrawRectangleRec(hostButton, GREEN);
            DrawText("HÔTE", hostButton.x + 75, hostButton.y + 20, 20, DARKGREEN);
        }

        DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 10, 170, 20, BLACK);
        DrawText("Liste des hotes :", 10, 200, 20, BLACK);

        for (int i = 0; i < hotes.nbUsers; i++) {
            Rectangle hostEntry = {10, 230 + (30 * i), 300, 20};
            DrawRectangleRec(hostEntry, GRAY);
            DrawText(hotes.tab[i].name, hostEntry.x + 5, hostEntry.y, 20, BLACK);
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, hostEntry)) {
                DrawRectangleRec(hostEntry, GREEN);
                DrawText(hotes.tab[i].name, hostEntry.x + 5, hostEntry.y, 20, DARKGREEN);
            }
        }

        DrawFPS(10, 430);

    EndDrawing();
}

#pragma endregion



#pragma region LOBBY

/**
 * \fn void renderLOBBY()
 * \brief Rendue de l'écran de lobby pour l'hôte (liste des joueurs connectés, bouton START)
 */
void renderLOBBY(){
    BeginDrawing();

        ClearBackground(SKYBLUE);

        DrawText("Salle d'attente - Hôte", 10, 10, 20, BLACK);

        Rectangle quitButton = {300, 100, 200, 60};
        DrawRectangleRec(quitButton, GRAY);
        DrawText("QUITTER", quitButton.x + 65, quitButton.y + 20, 20, BLACK);

        if(clients_app.nbUsers > 1){
            Rectangle startButton = {300, 180, 200, 60};
            DrawRectangleRec(startButton, GRAY);
            DrawText("START", startButton.x + 75, startButton.y + 20, 20, BLACK);
        }

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, quitButton)) {
            DrawRectangleRec(quitButton, RED);
            DrawText("QUITTER", quitButton.x + 65, quitButton.y + 20, 20, DARKGRAY);
        }

        if(clients_app.nbUsers > 1){
            Rectangle startButton = {300, 180, 200, 60};
            if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, startButton)) {
                DrawRectangleRec(startButton, GREEN);
                DrawText("START", startButton.x + 75, startButton.y + 20, 20, DARKGREEN);
            }
        }

        DrawText(TextFormat("IP serveur applicatif : %s:%hu", IP_SERVICE, PORT_SRV_APP), 10, 250, 20, BLACK);
        DrawText("Joueurs connectés :", 10, 280, 20, BLACK);
        
        for (int i=0; i<clients_app.nbUsers; i++) {
            DrawText(TextFormat("- %s", clients_app.tab[i].name), 40, 310 + (30*i), 20, BLACK);
        }

        DrawFPS(10, 450-20);

    EndDrawing();
}

#pragma endregion



#pragma region LOBBY Client

/**
 * \fn void renderLOBBYClt()
 * \brief Rendue de l'écran de lobby pour le client (liste des joueurs, bouton QUITTER)
 */
void renderLOBBYClt(){
    BeginDrawing();

        ClearBackground(SKYBLUE);

        DrawText(TextFormat("Bienvenue dans le serveur de [%s] !!", hote_serv_app.name), 10, 10, 20, BLACK);

        Rectangle quitButton = {300, 100, 200, 60};
        DrawRectangleRec(quitButton, GRAY);
        DrawText("QUITTER", quitButton.x + 65, quitButton.y + 20, 20, BLACK);

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        if (CheckCollisionPointRec((Vector2){mouse_x, mouse_y}, quitButton)) {
            DrawRectangleRec(quitButton, RED);
            DrawText("QUITTER", quitButton.x + 65, quitButton.y + 20, 20, DARKGRAY);
        }

        DrawText("Joueurs connectés :", 10, 200, 20, BLACK);
        
        for (int i=0; i<clients.nbUsers; i++) {
            DrawText(TextFormat("- %s", clients.tab[i].name), 40, 230 + (30*i), 20, BLACK);
        }

        DrawFPS(10, 450-20);

    EndDrawing();
}


#pragma endregion




#pragma region START

/**
 * \fn void renderSTART()
 * \brief Rendue de l'écran de départ avec un compte à rebours de 3 secondes avant le début de partie
 */
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

/**
 * \fn void renderGAME()
 * \brief Rendue de l'écran de jeu (scène 3D, balles, visée, IHM hôte, indicateur de tour)
 */
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

/**
 * \fn void renderEND()
 * \brief Rendue de l'écran de fin de partie
 */
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

/**
 * \fn void renderNEXT()
 * \brief Rendue de l'écran de transition entre deux manches avec compte à rebours
 */
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

/**
 * \fn void renderPODIUM()
 * \brief Rendue de l'écran du podium avec le tableau des scores par manche et le total
 */
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