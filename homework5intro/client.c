// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

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

  printf("after connecting to the server\n");
  return 0;
}
