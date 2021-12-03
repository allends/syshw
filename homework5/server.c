#include "common.h"
#include <sys/types.h>
#include <signal.h>

#define MAXIMUMPLAYERS 4
TILETYPE main_game_grid[GRIDSIZE][GRIDSIZE];
#define BUFFER_SZ sizeof(main_game_grid)
#define POSITION_BUFFER 3

Position playerPosition;
int score;
int level;
int numTomatoes;

bool shouldExit = false;

TTF_Font *font;

// keep data on what the tile is like and where the players are
// each player should have a score and stuff like that

typedef struct
{
  int player_id;
  int player_clientfd;
  pthread_t player_thread_id;
  Position player_position;
  bool connected;
} player_t;

player_t *players[MAXIMUMPLAYERS];
int players_connected = 0;

// get a random value in the range [0, 1]
double rand01()
{
  return (double)rand() / (double)RAND_MAX;
}

void initGrid()
{
  for (int i = 0; i < GRIDSIZE; i++)
  {
    for (int j = 0; j < GRIDSIZE; j++)

    {
      double r = rand01();
      if (r < 0.1)
      {
        main_game_grid[i][j] = TILE_TOMATO;
        numTomatoes++;
      }
      else
        main_game_grid[i][j] = TILE_GRASS;
    }
  }

  // force player's position to be grass
  for (int player_index = 0; player_index < MAXIMUMPLAYERS; player_index++)
  {
    if (!players[player_index])
    {
      continue;
    }
    int current_x = players[player_index]->player_position.x;
    int current_y = players[player_index]->player_position.y;

    if (main_game_grid[current_x][current_y] == TILE_TOMATO)
    {
      main_game_grid[current_x][current_y] = TILE_GRASS;
      numTomatoes--;
    }
  }

  // ensure grid isn't empty
  while (numTomatoes == 0)
    initGrid();
}

void serialize_game_data(char *destination, TILETYPE input[GRIDSIZE][GRIDSIZE])
{
  int index = 0;
  for (int i = 0; i < GRIDSIZE; i++)
  {
    for (int j = 0; j < GRIDSIZE; j++)
    {
      switch (input[i][j])
      {
      case TILE_GRASS:
        destination[index] = 'G';
        break;
      case TILE_TOMATO:
        destination[index] = 'T';
        break;
      case TILE_PLAYER:
        destination[index] = 'P';
        break;
      default:
        break;
      }
      index++;
    }
  }
}

int open_serverfd()
{
  int serverfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;

  bind(serverfd, (struct sockaddr *)&server_address, sizeof(server_address));

  listen(serverfd, 5);

  return serverfd;
}

void *handle_client(void *args)
{
  player_t *player = (player_t *)args;
  char data_received[POSITION_BUFFER];
  bzero(data_received, POSITION_BUFFER);

  while (player->connected)
  {
    int receive_result = recv(player->player_clientfd, data_received, POSITION_BUFFER, 0);
    printf("got data: %s with receive_result: %d\n", data_received, receive_result);
    if (receive_result == 0)
    {
      printf("client disconnected \n");
      break;
    }
    bzero(data_received, POSITION_BUFFER);
  }

  // free the memory that we allocated
  for (int index = 0; index < MAXIMUMPLAYERS; index++)
  {
    if (players[index]->player_id == player->player_id)
    {
      free(players[index]);
      players[index] = NULL;
      break;
    }
  }

  pthread_detach(player->player_thread_id);
  printf("closing the clients thread\n");
  players_connected--;
  return NULL;
}

void *send_game_data(void *args)
{
  char *serialized_game_data = (char *)malloc(GRIDSIZE * GRIDSIZE);
  while (1)
  {
    serialize_game_data(serialized_game_data, main_game_grid);
    for (int player_index = 0; player_index < MAXIMUMPLAYERS; player_index++)
    {
      // check to see if the clients file descriptor is valid
      if (players[player_index])
      {
        send(players[player_index]->player_clientfd, serialized_game_data, strlen(serialized_game_data), 0);
      }
    }
    sleep(1);
  }
  free(serialized_game_data);
  return NULL;
}

void *client_acceptor(void *args)
{
  int clientfd;
  struct sockaddr_in client_address;
  socklen_t client_address_length = sizeof(client_address);
  int serverfd = open_serverfd();

  pthread_t player_thread;
  signal(SIGPIPE, SIG_IGN);

  while (1)
  {
    clientfd = accept(serverfd, (struct sockaddr *)&client_address, &client_address_length);
    printf("a client had connected %d\n", clientfd);

    if (players_connected + 1 >= MAXIMUMPLAYERS)
    {
      printf("Not accepting new players\n");
      close(clientfd);
    }
    else
    {
      // increment the amount of players that we have
      // we will also need to update the sprite sheet
      // make a player struct for each other the connected players with their positions on there
      player_t *new_player = (player_t *)malloc(sizeof(player_t));
      new_player->player_clientfd = clientfd;
      new_player->player_id = players_connected;
      new_player->player_thread_id = player_thread;
      new_player->connected = true;
      Position new_pos;
      new_pos.x = new_pos.y = GRIDSIZE / 2;
      new_player->player_position = new_pos;
      players[players_connected] = new_player;
      players_connected++;

      // TODO: array for the players connected
      pthread_create(&player_thread, NULL, handle_client, (void *)new_player);
    }
  }

  close(serverfd);
  return NULL;
}

int main(int argc, char const *argv[])
{

  // lets see if we can make this its own thread to accept new players
  pthread_t client_acceptor_thread;
  pthread_t send_game_data_thread;
  pthread_create(&client_acceptor_thread, NULL, client_acceptor, NULL);
  pthread_create(&send_game_data_thread, NULL, send_game_data, NULL);
  // another thread to check if the players leave?

  // constantly reinitialize the grid so that the clients can see that
  while (1)
  {
    initGrid();
    sleep(1);
  }

  pthread_join(send_game_data_thread, NULL);
  pthread_join(client_acceptor_thread, NULL);
  return 0;
}