#include "common.h"

TILETYPE grid[GRIDSIZE][GRIDSIZE];

Position playerPosition;
int score;
int level;
int numTomatoes;

bool shouldExit = false;

TTF_Font *font;

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
        grid[i][j] = TILE_TOMATO;
        numTomatoes++;
      }
      else
        grid[i][j] = TILE_GRASS;
    }
  }

  // force player's position to be grass
  if (grid[playerPosition.x][playerPosition.y] == TILE_TOMATO)
  {
    grid[playerPosition.x][playerPosition.y] = TILE_GRASS;
    numTomatoes--;
  }

  // ensure grid isn't empty
  while (numTomatoes == 0)
    initGrid();
}

int main(int argc, char const *argv[])
{
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};
  char *hello = "Hello from server";

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                 &opt, sizeof(opt)))
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *)&address,
           sizeof(address)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

  while (read(new_socket, buffer, 1024) != 0)
  {
    printf("%d\n", buffer);
  }

  return 0;
}
