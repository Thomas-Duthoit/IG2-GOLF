# IG2-GOLF
Projet de Middleware Client/Serveur

# SOMMAIRE
- [Clonage du repository](#clonage-du-repository)
- [Dépendances](#dépendances)
- [Compilation](#compilation)
- [Mode d'emploi](#mode-demploi)
  - [1. Le serveur d'enregistrement](#1-le-serveur-denregistrement)
  - [2. Le client](#2-le-client)
  - [3. Héberger une partie](#3-héberger-une-partie)
  - [4. Rejoindre une partie](#4-rejoindre-une-partie)
  - [5. Contrôles en partie](#5-contrôles-en-partie)

---


# Clonage du repository
Comme le fichier utilise raylib en submodule, il faut cloner le repository en mode récursif afin de cloner [raylib](https://github.com/raysan5/raylib/) également
```sh
git clone --recurse-submodules https://github.com/Thomas-Duthoit/IG2-GOLF.git
```

# Dépendances

Pour la compilation sur Ubuntu, il faut les dépendances :
```sh
apt install build-essential git
apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```
Sur d'autres distributions, consultez le github de raylib afin d'installer les packages correspondants : 
> https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux


# Compilation

Dans l'archive du projet, la compilation se fait avec :

``` sh
make
```

# Mode d'emploi

## 1. Le serveur d'enregistrement

Le serveur est le premier exécutable à lancer pour jouer à IG2-GOLF. A partir du répertoire du projet, le serveur est lançable grâce à l'exécutable situé dans `bin/server.exe`, avec comme arguments l'adresse IP d'écoute (`0.0.0.0`) et le port d'écoute. 

## 2. Le client 

A partir du répertoire du projet, le client est lançable grâce à l'éxécutable situé dans `bin/client.exe`.

> ATTENTION : Pour lancer le client il faut préciser l'adresse IP du serveur d'enregistrement, le port de ce dernier et l'interface wifi de la machine : `bin/client.exe \<IP\> \<port\> \<interface\>

## 3. Héberger une partie

Pour qu’un client héberge une partie avec son serveur applicatif, il faut cliquer sur le bouton “HÔTE”.
Une fois tous les joueurs présents, le bouton “START” permet de lancer la partie pour tous les joueurs.
Pour ne plus héberger de partie, le bouton “QUITTER” permet de revenir à la vue d'accueil

## 4. Rejoindre une partie

Pour rejoindre une partie, il faut cliquer sur le bouton qui contient le pseudo de l’hôte dont on veut rejoindre la partie sur l’IHM.
Le bouton “QUITTER” permet de revenir au menu principal.

## 5. Contrôles en partie

Lors d'une partie, que vous soyez le serveur applicatif ou un client, il est possible de participer à la partie. 
Pour jouer, les boutons intéractifs sont les suivants : 
  - `[Z]` : Permet d’avancer la caméra en mode observation, 
  - `[S]` : Permet de reculer la caméra en mode observation, 
  - `[Q]` : Permet de déplacer la caméra à gauche en mode observation,
  - `[D]` : Permet de déplacer la caméra à droite en mode observation,
  - `[ESPACE]` : Permet de monter la caméra en mode observation,
  - `[CTRL]` : Permet de descendre la caméra en mode observation,
  - `[R]` : Permet de retourner à la balle de golf du joueur et de passer en mode tir,
  - `[Souris]` : Permet de tourner la caméra,
  - `[Clic gauche] + [Souris]`  : Permet de tirer si c’est à notre tour de jouer et qu’on est en mode tir (`[R]`).


