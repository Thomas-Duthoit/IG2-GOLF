# IG2-GOLF
Projet de Middleware Client/Serveur

# SOMMAIRE
- [<ins>**CLONAGE**</ins>](#clonage-du-repository)
- [<ins>**DEPENDANCES**</ins>](#dépendances)
- [<ins>**COMPILATION**</ins>](#compilation)
- [<ins>**MODE D'EMPLOI**</ins>](#mode-demploi)
    - [<ins>**I. Le serveur d'enregistrement**</ins>](#i-le-serveur-denregistrement)
    - [<ins>**II. Le client**</ins>](#ii-le-client)
    - [<ins>**III. En partie**</ins>](#iii-en-partie)

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

Le serveur est le premier exécutable à lancer pour jouer à IG2-GOLF. A partir du répertoire du projet, le serveur est lançable grâce à l'exécutable situé dans `bin/server.exe`. 

## 2. Le client 

A partir du répertoire du projet, le client est lançable grâce à l'éxécutable situé dans `bin/client.exe`.

> ATTENTION : Pour lancer le client il faut préciser l'adresse IP du serveur d'enregistrement, le port de ce dernier et l'interface wifi de la machine : `bin/client.exe \<IP\> \<port\> \<interface\>

## 3. En partie 

Lors d'une partie, que vous soyez le serveur applicatif ou un client, il est possible de participer à la partie. 
Pour jouer, les boutons intéractifs sont les suivants : 
  - `[Z]` : Permet d’avancer la “caméra” en mode observation, 
  - `[S]` : Permet de reculer la “caméra” en mode observation, 
  - `[Q]` : Permet de déplacer la “caméra” à gauche en mode observation,
  - `[D]` : Permet de déplacer la “caméra” à droite en mode observation,
  - `[ESPACE]` :  Permet de retourner à la balle de golf du joueur,
  - `[R]` :  Permet de tourner la "caméra” (tourne vers la droite).


