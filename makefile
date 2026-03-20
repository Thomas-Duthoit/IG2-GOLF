# pour la compilation sur ubuntu, il faut les dépendances :
#    sudo apt install build-essential git
#    sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
# sur les autres distributions :
#    https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux


CCFLAGS = -Iinclude/
LDFLAGS = -Llib/raylib/src -lraylib -lm	-ldl -lpthread -lX11


all: raylib bin/client

raylib:
	@echo "Compilation de raylib"
	cd lib/raylib/src && make PLATFORM=PLATFORM_DESKTOP


bin/client: src/client.c
	@echo "Compilation du client"
	mkdir -p build
	mkdir -p bin
	gcc $(CCFLAGS) src/client.c $(LDFLAGS) -o bin/client



clean:
	@echo "> Nettoyage du projet"
	cd lib/raylib/src && make clean

	rm bin/*