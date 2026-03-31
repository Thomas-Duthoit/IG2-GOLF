# pour la compilation sur ubuntu, il faut les dépendances :
#    sudo apt install build-essential git
#    sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
# sur les autres distributions :
#    https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux


CCFLAGS = -Iinclude/
LDFLAGS = -Llib/raylib/src -Llib/libINET/ -lraylib -lm	-ldl -lpthread -lX11 -lINET


all: raylib bin/client bin/server

raylib:
	@echo "Compilation de raylib"
	cd lib/raylib/src && make PLATFORM=PLATFORM_DESKTOP

session.o: src/session.c include/session.h
	@echo "Compilation de session.o"
	mkdir -p build
	gcc -c $(CCFLAGS) src/session.c -o build/session.o

data.o: src/data.c include/data.h
	@echo "Compilation de data.o"
	mkdir -p build
	gcc -c $(CCFLAGS) src/data.c -o build/data.o

libINET.a: session.o data.o
	@echo "Compilation de libINET.a"
	mkdir -p lib/libINET
	ar rcs lib/libINET/libINET.a build/session.o build/data.o


bin/client: src/client.c libINET.a src/reqRep.c src/dial.c src/users.c
	@echo "Compilation du client"
	mkdir -p build
	mkdir -p bin
	gcc $(CCFLAGS) src/client.c src/reqRep.c src/dial.c src/users.c $(LDFLAGS) -DCLIENT -o bin/client


bin/server: src/server.c libINET.a src/reqRep.c src/dial.c src/users.c
	@echo "Compilation du serveur"
	mkdir -p build
	mkdir -p bin
	gcc $(CCFLAGS) src/server.c src/reqRep.c src/dial.c src/users.c $(LDFLAGS) -DSERVER -o bin/server


clean:
	@echo "> Nettoyage du projet"
	cd lib/raylib/src && make clean

	rm bin/*
	rm build/*
	rm -f lib/libINET/libINET.a
