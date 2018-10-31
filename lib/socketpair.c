#include "threading.h"
#include "socketpair.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>

#define UNIXSOCK_PATH "/private/tmp/mysh.sock"
#define BUFFER_SIZE 1024
#define CLIENT_SOCK_MAX_RETRIES 10

int* create_unix_socketpair()
{
  int* retpair = NULL; // server_socket, serverside_clientsocket, clientside_clientsocket
  pthread_t pthread_srv, pthread_cli;

  if (run_pthread((void *)create_server_unix_socketpair, NULL, &pthread_srv))
  {
    if (run_pthread((void *)create_client_unix_socketpair, NULL, &pthread_cli))
    {      
      int retval_cli = wait_pthread_finishes(&pthread_cli);
      int retval_srv = wait_pthread_finishes(&pthread_srv);

      retpair = (int *)calloc(3, sizeof(int));
      memset(retpair, -1, sizeof(int));

      retpair[0] = retval_srv >> 16;
      retpair[1] = retval_srv & 0xFFFF;
      retpair[2] = retval_cli;
    }
    else fprintf(stderr, "failed to create serverside socket pthread\n");  
  }
  else fprintf(stderr, "failed to create serverside socket pthread\n");
  
  if (retpair == NULL || (retpair[0] != -1 && retpair[1] != -1 && retpair[2] != -1))
    return retpair;
  else return NULL;
}

int create_server_unix_socketpair()
{
  int success = 0;

  int s_socket_fd, c_socket_fd;
  struct sockaddr_un s_addr, c_addr;

  unsigned int c_addr_length = sizeof(c_addr);

  if (access(UNIXSOCK_PATH, F_OK) == 0)
    unlink(UNIXSOCK_PATH);

  if ((s_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) > -1)
  {
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sun_family = AF_UNIX;
    strcpy(s_addr.sun_path, UNIXSOCK_PATH);

    int stat = -1;

    if ((stat = bind(s_socket_fd, (const struct sockaddr *)&s_addr, sizeof(s_addr))) > -1)
    {
      if ((stat = listen(s_socket_fd, 1)) > -1) // backlog: 1
      {
        if ((c_socket_fd = accept(s_socket_fd, (struct sockaddr *)&c_addr, &c_addr_length)) > -1) // blocking
          success = 1;
        else perror("[s] accept failed");
      }
      else perror("[s] listen failed");
    }
    else perror("[s] bind failed");
  }
  else perror("[s] create socket failed");

  return success ? ((s_socket_fd << 16) | c_socket_fd) : -1;
}

int create_client_unix_socketpair()
{
  int success = 0;

  int c_socket_fd;  
  struct sockaddr_un c_addr;

  c_addr.sun_family = AF_UNIX;
  strcpy(c_addr.sun_path, UNIXSOCK_PATH);

  c_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  for (int i = 0; i < CLIENT_SOCK_MAX_RETRIES; i++)
  {
    if (connect(c_socket_fd, (const struct sockaddr *)&c_addr, sizeof(c_addr)) > -1)
    {
      success = 1;
      break;
    }

    usleep(10); // server should create socket in 10*CLIENT_SOCK_MAX_RETRIES milliseconds (def: 100ms)
  }

  return success ? c_socket_fd : -1;
}

int socketpair_receive(int socket_fd, int bufsize, char** out_data)
{
  char* buffer = (char*)calloc(bufsize + 1, sizeof(char));

  *out_data = buffer;

  return read(socket_fd, buffer, bufsize);
}

int socketpair_send(int socket_fd, const char* write_data)
{
  return send(socket_fd, write_data, strlen(write_data), 0);
}

int dispose_socketpair(int* socketpair)
{
  errno = 0;

  for (int i = 2; i > -1; i--)
    close(socketpair[i]); // clientside -> serverside -> server

  return errno ? 0 : 1;
}