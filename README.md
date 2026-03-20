# IG2-GOLF
Projet de Middleware Client/Serveur


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

