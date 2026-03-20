# IG2-GOLF
Projet de Middleware Client/Serveur


pour la compilation sur ubuntu, il faut les dépendances :
```
sudo apt install build-essential git
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```
sur les autres distributions, consulter le github de raylibb : 
> https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux


Compilation: \
Il faut télécharger raylib, puis on peut compiler le projet:
``` sh
cd lib/
git clone --depth 1 https://github.com/raysan5/raylib.git raylib
cd ..
make
```

