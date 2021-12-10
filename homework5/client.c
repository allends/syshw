#include "common.h"

TILETYPE game_grid[GRIDSIZE][GRIDSIZE];
#define BUFFER_SZ 32
#define GAME_BUFFER_SZ sizeof(grid)
#define POSITION_BUFFER_SZ 2

Position playerPosition;
int score;
int level;
int numTomatoes;
int mainfd; // for sending data from function
pthread_mutex_t send_data;

bool shouldExit = false;

TTF_Font *font;

// get a random value in the range [0, 1]
double rand01()
{
    return (double)rand() / (double)RAND_MAX;
}

void initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int rv = IMG_Init(IMG_INIT_PNG);
    if ((rv & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        fprintf(stderr, "Error initializing IMG: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1)
    {
        fprintf(stderr, "Error initializing TTF: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

char delta_to_char(int delta)
{
    switch (delta)
    {
    case 0:
        return 'N';
        break;
    case 1:
        return 'U';
        break;
    case -1:
        return 'D';
        break;
    default:
        return 'N';
        break;
    }
}

void send_delta_server(int delta_x, int delta_y)
{
    char data[2];
    data[0] = delta_to_char(delta_x);
    data[1] = delta_to_char(delta_y);

    send(mainfd, data, POSITION_BUFFER_SZ, 0);
}

void moveTo(int x, int y)
{
    // Prevent falling off the grid

    if (x == 0 && y == 0)
    {
        playerPosition.x = 0;
        playerPosition.y = 0;
        return;
    }

    // Sanity check: player can only move to 4 adjacent squares
    if (!(abs(x) == 1 && abs(y) == 0) &&
        !(abs(x) == 0 && abs(y) == 1))
    {
        fprintf(stderr, "Invalid move attempted from (%d, %d) to (%d, %d)\n", playerPosition.x, playerPosition.y, x, y);
        return;
    }

    // TODO create a function
    // send_delta(int x, int y)
    // that send the data right to the server, no flags, no mutexes

    send_delta_server(x, y);
    playerPosition.x = x;
    playerPosition.y = y;
}

void handleKeyDown(SDL_KeyboardEvent *event)
{

    // ignore repeat events if key is held down
    if (event->repeat)
        return;

    if (event->keysym.scancode == SDL_SCANCODE_Q || event->keysym.scancode == SDL_SCANCODE_ESCAPE)
    {
        shouldExit = true;
        printf("exiting from the key handler \n");
    }

    if (event->keysym.scancode == SDL_SCANCODE_UP || event->keysym.scancode == SDL_SCANCODE_W)
        moveTo(0, -1);

    if (event->keysym.scancode == SDL_SCANCODE_DOWN || event->keysym.scancode == SDL_SCANCODE_S)
        moveTo(0, 1);

    if (event->keysym.scancode == SDL_SCANCODE_LEFT || event->keysym.scancode == SDL_SCANCODE_A)
        moveTo(-1, 0);

    if (event->keysym.scancode == SDL_SCANCODE_RIGHT || event->keysym.scancode == SDL_SCANCODE_D)
        moveTo(1, 0);
}

void processInputs()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            printf("exiting from quit statement \n");
            shouldExit = true;
            break;

        case SDL_KEYDOWN:
            handleKeyDown(&event.key);
            break;

        default:
            break;
        }
    }
}

void drawGrid(SDL_Renderer *renderer, SDL_Texture *grassTexture, SDL_Texture *tomatoTexture, SDL_Texture *playerTexture)
{
    SDL_Rect dest;
    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            dest.x = 64 * i;
            dest.y = 64 * j + HEADER_HEIGHT;
            SDL_Texture *texture;
            switch (game_grid[i][j])
            {
            case TILE_GRASS:
                texture = grassTexture;
                break;
            case TILE_TOMATO:
                texture = tomatoTexture;
                break;
            case TILE_PLAYER:
                texture = grassTexture;
                SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                texture = playerTexture;
                break;
            }
            SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, texture, NULL, &dest);
        }
    }
}

void drawUI(SDL_Renderer *renderer)
{
    // largest score/level supported is 2147483647
    char scoreStr[18];
    char levelStr[18];
    sprintf(scoreStr, "Score: %d", score);
    sprintf(levelStr, "Level: %d", level);

    SDL_Color white = {255, 255, 255};
    SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreStr, white);
    SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

    SDL_Surface *levelSurface = TTF_RenderText_Solid(font, levelStr, white);
    SDL_Texture *levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);

    SDL_Rect scoreDest;
    TTF_SizeText(font, scoreStr, &scoreDest.w, &scoreDest.h);
    scoreDest.x = 0;
    scoreDest.y = 0;

    SDL_Rect levelDest;
    TTF_SizeText(font, levelStr, &levelDest.w, &levelDest.h);
    levelDest.x = GRID_DRAW_WIDTH - levelDest.w;
    levelDest.y = 0;

    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreDest);
    SDL_RenderCopy(renderer, levelTexture, NULL, &levelDest);

    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    SDL_FreeSurface(levelSurface);
    SDL_DestroyTexture(levelTexture);
}

void deserialize_game_data(char inputstream[GRIDSIZE * GRIDSIZE])
{
    int index = 0;
    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            switch (inputstream[index])
            {
            case 'G':
                game_grid[i][j] = TILE_GRASS;
                break;
            case 'T':
                game_grid[i][j] = TILE_TOMATO;
                break;
            case 'P':
                game_grid[i][j] = TILE_PLAYER;
                break;
            default:
                break;
            }
            index++;
        }
    }
}

void *receive_server_data(void *args)
{
    int buffer_length = GRIDSIZE * GRIDSIZE;
    char received_serialized_game_data[buffer_length];
    bzero(received_serialized_game_data, buffer_length);

    int *serverfd = (int *)args;

    while (!shouldExit)
    {
        // The client is getting hung up here and cant close
        int receive_result = recv(*serverfd, received_serialized_game_data, buffer_length, 0);
        if (receive_result > 0)
        {
            printf("Game Data: %s \n", received_serialized_game_data);
            deserialize_game_data(received_serialized_game_data);
        }
        else
        {
            // this means that the server is no longer connected:
            shouldExit = true;
            break;
        }
        bzero(received_serialized_game_data, buffer_length);
    }
    printf("closing the server receiver\n");
    return NULL;
}

int open_clientfd()
{
    int clientfd;
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    mainfd = clientfd;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int result = connect(clientfd, (struct sockaddr *)&server_address, sizeof(server_address));
    return clientfd;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    level = 1;

    initSDL();

    font = TTF_OpenFont("resources/Burbank-Big-Condensed-Bold-Font.otf", HEADER_HEIGHT);
    if (font == NULL)
    {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if (window == NULL)
    {
        fprintf(stderr, "Error creating app window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL)
    {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Texture *grassTexture = IMG_LoadTexture(renderer, "resources/grass.png");
    SDL_Texture *tomatoTexture = IMG_LoadTexture(renderer, "resources/tomato.png");
    SDL_Texture *playerTexture = IMG_LoadTexture(renderer, "resources/player.png");

    // networking stuff
    int clientfd = open_clientfd();

    pthread_t receive_server_data_thread;
    pthread_create(&receive_server_data_thread, NULL, receive_server_data, &clientfd);

    // end networking setup

    // main game loop (maybe put into its own thread)
    while (!shouldExit)
    {
        SDL_SetRenderDrawColor(renderer, 0, 105, 6, 255);
        SDL_RenderClear(renderer);

        processInputs();

        drawGrid(renderer, grassTexture, tomatoTexture, playerTexture);
        drawUI(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // 16 ms delay to limit display to 60 fps
    }

    // clean up everything
    pthread_join(receive_server_data_thread, NULL);

    SDL_DestroyTexture(grassTexture);
    SDL_DestroyTexture(tomatoTexture);
    SDL_DestroyTexture(playerTexture);

    TTF_CloseFont(font);
    TTF_Quit();

    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
