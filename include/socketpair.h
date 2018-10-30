#ifndef SOCKETPAIR_H_
#define SOCKETPAIR_H_

int* create_unix_socketpair();
int create_server_unix_socketpair();
int create_client_unix_socketpair();
int dispose_socketpair(int* socketpair);

#endif