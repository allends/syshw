#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 8080 // has to be the same for the client and the server
#define MAXIMUMPLAYERS 4

#define BUFFER_SZ 1024

// keep data on what the tile is like and where the players are
// each player should have a score and stuff like that

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

typedef struct
{
  int player_id;
  int player_clientfd;
  pthread_t player_thread_id;
} player_t;

player_t *players[MAXIMUMPLAYERS];
int players_connected = 0;

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
  char buffer[BUFFER_SZ];
  bzero(buffer, BUFFER_SZ);

  while (1)
  {
    int receive_result = recv(player->player_clientfd, buffer, BUFFER_SZ, 0);
    if (receive_result > 0)
    {
      if (strlen(buffer) > 0)
      {
        printf("received: %s - from cliend %d\n", buffer, player->player_id);
      }
    }
    else
    {
      break;
    }
    bzero(buffer, BUFFER_SZ);
  }
  pthread_detach(player->player_thread_id);
  printf("closing the clients thread\n");
  players_connected--;
  return NULL;
}

void serialize_game_data(char *destination, TILETYPE input[3][3])
{
  int index = 0;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      printf("char at index %d is %c \n", index, input[i][j]);
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
  destination[9] = '\0';
  for (int i = 0; i < 9; i++)
  {
    printf("%d -> %c \n", i, destination[i]);
  }
  printf("%s \n", destination);
}

void *send_game_data(void *args)
{
  // serialize the data in the array and then send it over
  TILETYPE data[3][3] = {
      {TILE_GRASS, TILE_GRASS, TILE_TOMATO},
      {TILE_GRASS, TILE_GRASS, TILE_TOMATO},
      {TILE_GRASS, TILE_PLAYER, TILE_TOMATO}};

  char *destination = malloc(sizeof(char) * 10);

  serialize_game_data(destination, data);

  while (1)
  {
    for (int player_index = 0; player_index < MAXIMUMPLAYERS; player_index++)
    {
      // check to see if the clients file descriptor is valid
      if (players[player_index])
      {
        send(players[player_index]->player_clientfd, destination, strlen(destination), 0);
      }
    }
    sleep(1);
  }

  free(destination);
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

  pthread_join(send_game_data_thread);
  pthread_join(client_acceptor_thread);
  return 0;
}
