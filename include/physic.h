/**
 * \file physic.h
 * \brief Déclarations pour la physique du jeu
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#ifndef PHYSIC_H
#define PHYSIC_H

/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "map.h"

/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */

/**
 * \def MAX_PUISSANCE
 * \brief Puissance maximale du tir
 */
#define MAX_PUISSANCE 13.0f

/**
 * \def BALL_RADIUS
 * \brief Rayon de la balle
 */
#define BALL_RADIUS 0.1     // Rayon de la balle

/**
 * \def G
 * \brief Constante de gravité
 */
#define G 9.8               // constante de gravité

/**
 * \def KGRAD
 * \brief Constante d'accélération sur les pentes
 */
#define KGRAD 1             // constante accélération pente

/**
 * \def KVREBOND
 * \brief Vélocité minimale en Y pour un rebond
 */
#define KVREBOND 0.2        // velocité minimale en y pour un revond (< 0)

/**
 * \def EPSILON_GRAD
 * \brief Seuil pour les gradients
 */
#define EPSILON_GRAD 0.1

/**
 * \def EPSILON_VEL
 * \brief Seuil pour les vélocités
 */
#define EPSILON_VEL 0.01f

/**
 * \def KPENDUL
 * \brief Force du balancier
 */
#define KPENDUL 15.0f       // force du balancier

/**
 * \def RAYON_TROU
 * \brief Rayon de détection du trou
 */
#define RAYON_TROU 1.5f     // rayon de détection du trou

/**
 * \def DIS_HOLE_OK
 * \brief Distance horizontale OK avec le trou
 */
#define DIS_HOLE_OK 0.3f    // distance horizontal OK avec le trou

/**
 * \def VEL_HOLE_OK
 * \brief Vélocité OK pour entrer dans le trou
 */
#define VEL_HOLE_OK 0.3f    // vélocité OK pour entrer dans le trou

/**
 * \struct ball_t
 * \brief Structure représentant une balle
 */
typedef struct{
    Vector3 pos; /**< Position de la balle */
    Vector3 vel; /**< Vélocité de la balle */
    float ral; /**< Ralentissement de la balle */
    bool inAir; /**< Dans les airs */
    bool inMovement; /**< En mouvement */
    bool inHole; /**< Dans le trou */
} ball_t;

/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

/**
 * \fn void init_pos_ball(ball_t * ball, float startX, float startZ, float startY)
 * \brief Initialise la position et l'état d'une balle sur la carte
 * \param ball   Pointeur vers la balle à initialiser
 * \param startX Position X de départ
 * \param startZ Position Z de départ
 * \param startY Position Y de départ (hauteur du sol au point de départ)
 */
void init_pos_ball(ball_t * ball, float startX, float startZ, float startY);

/**
 * \fn void update_ball_mov(ball_t * ball, double dt, map_t * map)
 * \brief Met à jour le mouvement de la balle pour une frame (gravité, collisions, friction)
 * \param ball Pointeur vers la balle
 * \param dt   Delta temps de la frame (en secondes)
 * \param map  Pointeur vers la carte (pour les informations de sol)
 */
void update_ball_mov(ball_t * ball, double dt, map_t * map);

/**
 * \fn void isInHole(ball_t * ball, map_t * map)
 * \brief Vérifie si la balle est suffisamment proche et lente pour être considérée dans le trou
 * \param ball Pointeur vers la balle
 * \param map  Pointeur vers la carte
 */
void isInHole(ball_t * ball, map_t * map);

#endif // PHYSIC_H
