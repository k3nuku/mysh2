#include "socketpair.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  int state;

  int* ret = create_unix_socketpair();
  assert(ret);
  printf("[all] socketpair created success, %d %d %d\n",
    ret[0], ret[1], ret[2]);

  int s_socket = ret[0];
  int sc_socket = ret[1];
  int cc_socket = ret[2];

  // server -> client test
  state = socketpair_send(sc_socket, "testdatatoserver");
  assert(state > 0);
  printf("[s-c] %d bytes sent\n", state);

  char* buf;

  state = socketpair_receive(cc_socket, 1024, &buf);
  assert(state > 0);
  printf("[s-c] %d bytes, received string %s\n", state, buf);
  free(buf);

  // client -> server test
  state = socketpair_send(cc_socket, "testdatatoclient");
  assert(state > 0);
  printf("[c-s] %d bytes sent\n", state);

  state = socketpair_receive(sc_socket, 1024, &buf);
  assert(state > 0);
  printf("[c-s] %d bytes, received string %s\n", state, buf);
  free(buf);

  // closing test
  state = dispose_socketpair(ret);
  assert(state > 0);
  printf("[all] closing socket completed\n");

  return 0;
}
