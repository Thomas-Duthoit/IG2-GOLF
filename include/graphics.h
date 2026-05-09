/**
 * \file graphics.h
 * \brief Déclarations pour les graphismes et rendu
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H


/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "map.h"
#include "physic.h"
#include "users.h"

/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */

/**
 * \var camera
 * \brief Caméra 3D pour le rendu
 */
extern Camera3D camera;


/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

/**
 * \fn void init_graphics(map_t *maps)
 * \brief Initialise les graphismes
 * \param maps Tableau des cartes
 */
void init_graphics(map_t *maps);

/**
 * \fn void render_current_map(map_t *maps, int map_idx)
 * \brief Rendue de la carte actuelle
 * \param maps Tableau des cartes
 * \param map_idx Index de la carte sélectionnée
 */
void render_current_map(map_t *maps, int map_idx);

/**
 * \fn void render_ball(ball_t * ball, int player_index)
 * \brief Rendue d'une balle
 * \param ball Pointeur vers la balle
 * \param player_index Index du joueur
 */
void render_ball(ball_t * ball, int player_index);

/**
 * \fn void render_ball_name(ball_t * ball, name_t player_name)
 * \brief Rendue 2D du nom du joueur au-dessus de la balle en 3D
 * \param ball Pointeur vers la balle
 * \param player_name Nom du joueur
 */
void render_ball_name(ball_t * ball, name_t player_name);





#endif  // GRAPHICS_H