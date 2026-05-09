/**
 * \file map.h
 * \brief Déclarations pour la gestion des cartes
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#ifndef MAP_H
#define MAP_H

/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */

#include "raylib/raylib.h"

/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */

/**
 * \def MAX_HEIGHTMAP_DIMENSION
 * \brief Dimension maximale de la heightmap
 */
#define MAX_HEIGHTMAP_DIMENSION 60

/**
 * \def Y_SCALE
 * \brief Échelle Y pour la hauteur
 */
#define Y_SCALE 0.5

/**
 * \def MAX_MAPS
 * \brief Nombre maximum de cartes
 */
#define MAX_MAPS 3

/**
 * \struct map_t
 * \brief Structure représentant une carte
 */
typedef struct {

    int width; /**< Largeur de la carte */
    int height; /**< Hauteur de la carte */

    float heightmap[MAX_HEIGHTMAP_DIMENSION][MAX_HEIGHTMAP_DIMENSION]; /**< Heightmap de la carte */

    float hole_x; /**< Position X du trou */
    float hole_z; /**< Position Z du trou */

    float start_x; /**< Position X de départ */
    float start_z; /**< Position Z de départ */

    Mesh mesh; /**< Mesh de la carte */

} map_t;

/**
 * \struct ground_info_t
 * \brief Informations sur le sol à une position donnée
 */
typedef struct {
    float x;        /**< Position X world */
    float y;        /**< Hauteur interpolée (Y world) */
    float z;        /**< Position Z world */
    float grad_x;   /**< Pente selon l'axe X */
    float grad_z;   /**< Pente selon l'axe Z */
    Vector3 normal; /**< Vecteur normal du sol à ce point précis */
} ground_info_t;


/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

/**
 * \fn void load_map(map_t * map, char * path)
 * \brief Charge une carte depuis un fichier
 * \param map Pointeur vers la structure map
 * \param path Chemin du fichier
 */
void load_map(map_t * map, char * path);

/**
 * \fn Model get_map_model(map_t * map)
 * \brief Obtient le modèle 3D de la carte
 * \param map : Pointeur vers la structure map
 * \return Modèle Raylib
 */
Model get_map_model(map_t * map);

/**
 * \fn ground_info_t get_ground_info(map_t *map, float x, float z)
 * \brief Obtient les informations du sol à une position donnée
 * \param map : Pointeur vers la structure map
 * \param x : Coordonnée X
 * \param z : Coordonnée Z
 * \return Structure ground_info_t
 */
ground_info_t get_ground_info(map_t *map, float x, float z);

/**
 * \fn Mesh gen_map_borders(map_t *map, float baseHeight)
 * \brief Génère les bordures de la carte
 * \param map Pointeur vers la structure map
 * \param baseHeight Hauteur de base
 * \return Mesh des bordures
 */
Mesh gen_map_borders(map_t *map, float baseHeight);

/**
 * \fn void render_map(Model water_model, Model map_model, Model border_model, map_t * map)
 * @brief Rendue de la map pour l'affichage
 * @param water_model Modèle d'eau
 * @param map_model Modèle de carte
 * @param border_model Modèle de bordures
 * @param map Pointeur vers la structure map
 */
void render_map(Model water_model, Model map_model, Model border_model, map_t * map);

#endif  // MAP_H