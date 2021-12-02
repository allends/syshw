#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080 // has to be the same for the client and the server
#define MAXIMUMPLAYERS 4

// keep data on what the tile is like and where the players are
// each player should have a score and stuff like that

typedef struct
{
  int player_id;
  int player_clientfd;
} player_t;

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

void *handle_client(player_t player)
{
}

int main(int argc, char const *argv[])
{
  int clientfd;
  struct sockaddr_in client_address;
  socklen_t client_address_length = sizeof(client_address);
  int serverfd = open_serverfd();

  int players_connected = 0;

  while (1)
  {
    clientfd = accept(serverfd, (struct sockaddr *)&client_address, &client_address_length);
    printf("a client had connected\n");

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
      players_connected++;
    }
  }

  close(serverfd);
  return 0;
}
