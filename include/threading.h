#ifndef THREADING_H_
#define THREADING_H_

#include <pthread.h>

struct pthread_pool
{
  int thread_index;   // thread index for identify background process
  int* std_socket_fd; // socket fd for pthread sdio (server-client socket)

  pthread_t* head;    // link to head
  pthread_t* next;    // link to next thread
  pthread_t* thread;  // link to current thread
} pthread_pool_t;

int run_pthread(void* thread_instance, void* arguments_to_pass, pthread_t* out_pthread);
int wait_pthread_finishes(pthread_t* thread);

int process_bgcommand(char** argv);
int process_fgcommand(char** argv, int argc);
int process_pipecommand(char** argv);
int process_pipelining(char** argv, int argc);

int execute_command(char** argv, int is_pipecomm, int** last_pair, int* pair, int last_command);

void thread_bgcomm_execute(void* arg);
void thread_pipe_execute(void* arg);
void thread_wait_child(void* arg);

#endif