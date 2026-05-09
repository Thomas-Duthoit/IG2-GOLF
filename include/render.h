/**
 * \file render.h
 * \brief Déclaration pour le rendu des différents états du jeu (LIST, LOBBY, GAME, PODIUM, etc.)
 * \author Thomas DUTHOIT && Cloé GREBERT
 * \date 9 mai 2026
 * \version 1.0
 */

#ifndef RENDER_H
#define RENDER_H

/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

/**
 * \fn void renderSTART()
 * \brief Rendue de l'écran de départ avec un compte à rebours de 3 secondes avant le début de partie
 */
void renderSTART();

/**
 * \fn void renderList()
 * \brief Rendue de l'écran de liste des hôtes disponibles
 */
void renderLIST();

/**
 * \fn void renderLOBBY()
 * \brief Rendue de l'écran de lobby pour l'hôte (liste des joueurs connectés, bouton START)
 */
void renderLOBBY();

/**
 * \fn void renderLOBBYClt()
 * \brief Rendue de l'écran de lobby pour le client (liste des joueurs, bouton QUITTER)
 */
void renderLOBBYClt();

/**
 * \fn void renderGAME()
 * \brief Rendue de l'écran de jeu (scène 3D, balles, visée, IHM hôte, indicateur de tour)
 */
void renderGAME();

/**
 * \fn void renderEND()
 * \brief Rendue de l'écran de fin de partie
 */
void renderEND();

/**
 * \fn void renderNEXT()
 * \brief Rendue de l'écran de transition entre deux manches avec compte à rebours
 */
void renderNEXT();

/**
 * \fn void renderPODIUM()
 * \brief Rendue de l'écran du podium avec le tableau des scores par manche et le total
 */
void renderPODIUM();

#endif  // RENDER_H