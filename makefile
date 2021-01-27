build: demo/*.c src/*.h src/*.c src/utilities/*.c src/utilities/*.h
	gcc demo/*.c src/*.c src/utilities/*.c -o bin/3dtest -lSDL2 -lm

debug: demo/*.c src/*.h src/*.c src/utilities/*.c src/utilities/*.h
	gcc demo/*.c src/*.c src/utilities/*.c -o bin/3dtest -lSDL2 -lm -g -DDEBUGLEVEL=2