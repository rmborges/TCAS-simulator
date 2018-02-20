all: main

main: main.c dyn_broad.c listener.c aircraft.c tcas.c coord.c graphics.c sound.c general.h
	gcc -g -Wall -std=gnu99 -pedantic -o main main.c dyn_broad.c listener.c aircraft.c tcas.c coord.c graphics.c sound.c -lm -lrt -lpthread -lg2 -lvlc
