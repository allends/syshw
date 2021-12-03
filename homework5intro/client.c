// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define PORT 8080
#define BUFFER_SZ 32

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

TILETYPE main_grid[3][3];

void deserialize_game_data(char inputstream[10])
{
  int index = 0;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      switch (inputstream[index])
      {
      case 'G':
        main_grid[i][j] = TILE_GRASS;
        break;
      case 'T':
        main_grid[i][j] = TILE_TOMATO;
        break;
      case 'P':
        main_grid[i][j] = TILE_PLAYER;
        break;
      default:
        break;
      }
      index++;
    }
  }
}

void print_game_data(TILETYPE grid[3][3])
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      switch (grid[i][j])
      {
      case TILE_GRASS:
        printf("Grass\n");
        break;
      case TILE_TOMATO:
        printf("Tomato\n");
        break;
      default:
        printf("Player\n");
        break;
      }
    }
  }
  printf("----------------\n");
}

void *receive_server_data(void *args)
{
  char buffer[10];
  bzero(buffer, sizeof(buffer));

  int *serverfd = (int *)args;

  while (1)
  {
    int receive_result = recv(*serverfd, buffer, sizeof(buffer), 0);
    if (receive_result > 0)
    {
      printf("received: %s \n", buffer);
      deserialize_game_data(buffer);
      print_game_data(main_grid);
    }
    else
    {
      break;
    }
    bzero(buffer, sizeof(buffer));
  }
  return NULL;
}

void *send_server_data(void *args)
{
  char *message = "hello from the client";
  int *clientfd = (int *)args;

  while (1)
  {
    send(*clientfd, message, strlen(message), 0);
    sleep(1);
  }
}

int open_clientfd()
{
  int clientfd;
  clientfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int result = connect(clientfd, (struct sockaddr *)&server_address, sizeof(server_address));
  return clientfd;
}

int main(int argc, char const *argv[])
{
  int clientfd = open_clientfd();

  pthread_t send_server_data_thread;
  pthread_t receive_server_data_thread;
  pthread_create(&send_server_data_thread, NULL, send_server_data, &clientfd);
  pthread_create(&receive_server_data_thread, NULL, receive_server_data, &clientfd);

  pthread_join(receive_server_data_thread, NULL);
  pthread_join(send_server_data_thread, NULL);
  return 0;
}
