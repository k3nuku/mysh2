#include "threading.h"
#include "socketpair.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>

#define UNIXSOCK_NAME "/tmp/mysh"
#define UNIXSOCK_EXT ".sock"

#define CLIENT_SOCK_MAX_RETRIES 10

static int sock_count = 0;

int* create_unix_socketpair()
{
  int* retpair = NULL; // server_socket, serverside_clientsocket, clientside_clientsocket
  pthread_t pthread_srv, pthread_cli;

  if (run_pthread((void *)create_server_unix_socketpair, NULL, &pthread_srv))
  {
    if (run_pthread((void *)create_client_unix_socketpair, NULL, &pthread_cli))
    {
      int join_status_srv = 0;
      int join_status_cli = 0;

      int retval_cli;
      join_status_srv = pthread_join(pthread_cli, (void**)&retval_cli);

      int retval_srv;
      join_status_cli = pthread_join(pthread_srv, (void**)&retval_srv);

      if (join_status_cli || join_status_srv)
        fprintf(stderr, "error occured while join client and server thread\n");
      else
      {
        retpair = (int *)calloc(3, sizeof(int));
        memset(retpair, -1, sizeof(int));

        retpair[0] = retval_srv >> 16;
        retpair[1] = retval_srv & 0xFFFF;
        retpair[2] = retval_cli;

        sock_count++; // not to make duplicated socket fd
      }
    }
    else fprintf(stderr, "failed to create clientside socket pthread\n");  
  }
  else
  {
    fprintf(stderr, "failed to create serverside socket pthread, killing server thread\n");
    kill_pthread(pthread_srv);
  }

  if (!retpair)
    return NULL;
  else
  {
    if (retpair[0] != -1 && retpair[1] != -1 && retpair[2] != -1)
      return retpair;
    else return NULL;
  }
}

int create_server_unix_socketpair()
{
  int success = 0;

  int s_socket_fd, c_socket_fd;
  struct sockaddr_un s_addr, c_addr;

  unsigned int c_addr_length = sizeof(c_addr);

  bzero(&s_addr, sizeof(s_addr));

  char cnt_string[255];
  sprintf(cnt_string, "%d", sock_count);

  strcpy(s_addr.sun_path, UNIXSOCK_NAME);
  strcat(s_addr.sun_path, cnt_string);
  strcat(s_addr.sun_path, UNIXSOCK_EXT);

  if (access(s_addr.sun_path, F_OK) == 0)
    unlink(s_addr.sun_path);

  if ((s_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) > -1)
  {
    s_addr.sun_family = AF_UNIX;

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

  char cnt_string[10];
  sprintf(cnt_string, "%d", sock_count);

  strcpy(c_addr.sun_path, UNIXSOCK_NAME);
  strcat(c_addr.sun_path, cnt_string);
  strcat(c_addr.sun_path, UNIXSOCK_EXT);

  c_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  for (int i = 0; i < CLIENT_SOCK_MAX_RETRIES; i++)
  {
    if (connect(c_socket_fd, (const struct sockaddr *)&c_addr, sizeof(c_addr)) > -1)
    {
      success = 1;
      break;
    }

    usleep(10); // server thread should create socket in 10*CLIENT_SOCK_MAX_RETRIES milliseconds (def: 100ms)
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
    close(socketpair[i]); // clientside -> serverside -> server (order-sensitive)

  free(socketpair);

  return errno ? 0 : 1;
}