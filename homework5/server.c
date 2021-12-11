#include "common.h"
#include <sys/types.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>

#define MAXIMUMPLAYERS 4
TILETYPE main_game_grid[GRIDSIZE][GRIDSIZE];
#define POSITION_BUFFER 3
sem_t update_players_queue;

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
  int score;
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

int msleep(long msec)
{
  struct timespec ts;
  int res;

  if (msec < 0)
  {
    errno = EINVAL;
    return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do
  {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);

  return res;
}

void initGrid()
{
  // numTomatoes = 0;
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
  {
    printf("forced to reinitialize \n");
    initGrid();
  }
}

// This will add the players score and the level number
void add_player_data(char* destination, player_t* player){
  int score = player->score;
  char score_level_serialized[SCORE_SZ+LEVEL_SZ];
  // we are going to have a number 0-100
  // we can just turn it into a string - WOW
  sprintf(score_level_serialized, "%4d %1d", score, level);
  // make the end of the buffer have the players score
  for(int i=GRIDSIZE_LIN; i<GAME_DATA_BUFFER_SZ;i++){
    destination[i] = score_level_serialized[i-GRIDSIZE_LIN];
  }
  printf("ss: %s\n", destination);
}

// creates a string of game tiles that we can send to the client
void serialize_game_data(char *destination, TILETYPE input[GRIDSIZE][GRIDSIZE])
{
  for (int k = 0; k < MAXIMUMPLAYERS; k++)
  {
    if (players[k])
    {
      Position temp = players[k]->player_position;
      main_game_grid[temp.x][temp.y] = TILE_PLAYER;
    }
  }
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
  printf("serialized the game data \n");
}

int open_serverfd()
{
  int serverfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int bind_result = bind(serverfd, (struct sockaddr *)&server_address, sizeof(server_address));
  if(bind_result < 0){
    printf("binding error.\n");
  }

  int listen_result = listen(serverfd, 5);
  if(listen_result < 0){
    printf("listen error.\n");
  }

  return serverfd;
}

int char_to_move(char input)
{
  switch (input)
  {
  case 'N':
    return 0;
    break;
  case 'U':
    return 1;
    break;
  case 'D':
    return -1;
    break;
  default:
    return 0;
    break;
  }
}

void move_client(player_t *player, char data[2])
{
  int new_x = player->player_position.x + char_to_move(data[0]);
  int new_y = player->player_position.y + char_to_move(data[1]);
  if(main_game_grid[new_x][new_y] == TILE_PLAYER){
    sem_post(&update_players_queue);
    return;
  }
  if(main_game_grid[new_x][new_y] == TILE_TOMATO){
    player->score += 1;
    printf("player %d got a point! \n", player->player_id);
    numTomatoes--;
    if(numTomatoes == 0){
      initGrid();
      level++;
    }
  }
  int old_x = player->player_position.x;
  int old_y = player->player_position.y;
  player->player_position.x = new_x;
  player->player_position.y = new_y;
  main_game_grid[old_x][old_y] = TILE_GRASS;
  sem_post(&update_players_queue);
  return;
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
    if (receive_result <= 0)
    {
      printf("client disconnected \n");
      break;
    }
    else
    {
      move_client(player, data_received);
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
  //update the tile so new players wont see the disconnected avatar
  main_game_grid[player->player_position.x][player->player_position.y] = TILE_GRASS;
  pthread_detach(player->player_thread_id);
  printf("closing the clients thread\n");
  players_connected--;
  return NULL;
}

void *send_game_data(void *args)
{
  char *serialized_game_data = (char *)malloc(GAME_DATA_BUFFER_SZ);
  while (1)
  {
    sem_wait(&update_players_queue);
    serialize_game_data(serialized_game_data, main_game_grid);
    for (int player_index = 0; player_index < MAXIMUMPLAYERS; player_index++)
    {
      // check to see if the clients file descriptor is valid
      if (players[player_index])
      {
        add_player_data(serialized_game_data, players[player_index]);
        int send_result = send(players[player_index]->player_clientfd, serialized_game_data, strlen(serialized_game_data), 0);
        if(send_result == -1){
          printf("send error.\n");
        }
      }
    }
  }
  free(serialized_game_data);
  return NULL;
}

void create_player(int clientfd, pthread_t player_thread){
    player_t *new_player = (player_t *)malloc(sizeof(player_t));
    new_player->player_clientfd = clientfd;
    new_player->player_id = players_connected;
    new_player->player_thread_id = player_thread;
    new_player->connected = true;
    new_player->score = 0;
    Position new_pos;
    new_pos.x = (int) (rand01() * 10.0);
    new_pos.y = (int) (rand01() * 10.0);
    while(main_game_grid[new_pos.y][new_pos.y] == TILE_PLAYER){
      new_pos.x = (int) (rand01() * 10.0);
      new_pos.y = (int) (rand01() * 10.0);
    }
    new_player->player_position = new_pos;
    players[players_connected] = new_player;
    players_connected++;

    // TODO: array for the players connected
    pthread_create(&player_thread, NULL, handle_client, (void *)new_player);
    sem_post(&update_players_queue);
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
    printf("ready to accept a new client.\n");
    clientfd = accept(serverfd, (struct sockaddr *)&client_address, &client_address_length);
    printf("a client had connected %d\n", clientfd);

    if (players_connected >= MAXIMUMPLAYERS)
    {
      printf("Not accepting new players\n");
      close(clientfd);
    }
    else
    {
      create_player(clientfd, player_thread);
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
  sem_init(&update_players_queue, 1, 1);
  pthread_create(&client_acceptor_thread, NULL, client_acceptor, NULL);
  pthread_create(&send_game_data_thread, NULL, send_game_data, NULL);
  // another thread to check if the players leave?

  // constantly reinitialize the grid so that the clients can see that
  initGrid();

  pthread_join(send_game_data_thread, NULL);
  pthread_join(client_acceptor_thread, NULL);
  return 0;
}