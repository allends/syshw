#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define PORT 8080 // has to be the same for the client and the server

// Dimensions for the drawn grid (should be GRIDSIZE * texture dimensions)
#define GRID_DRAW_WIDTH 640
#define GRID_DRAW_HEIGHT 640

#define WINDOW_WIDTH GRID_DRAW_WIDTH
#define WINDOW_HEIGHT (HEADER_HEIGHT + GRID_DRAW_HEIGHT)

// Header displays current score
#define HEADER_HEIGHT 50

// Number of cells vertically/horizontally in the grid
#define GRIDSIZE 10
#define GRIDSIZE_LIN GRIDSIZE*GRIDSIZE
// 5 for the score and 2 for the level
#define SCORE_SZ 5
#define LEVEL_SZ 2
#define GAME_DATA_BUFFER_SZ GRIDSIZE_LIN + SCORE_SZ + LEVEL_SZ
#define MAXLEVELS 1
#define WINCODE 1337

typedef struct
{
    int x;
    int y;
} Position;

typedef enum
{
    TILE_GRASS,
    TILE_TOMATO,
    TILE_PLAYER
} TILETYPE;