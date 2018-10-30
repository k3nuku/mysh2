#ifndef THREADING_H_
#define THREADING_H_

#include <pthread.h>

struct pthread_pool
{
  int thread_index; // thread index for print number of background process

  pthread_t* head;
  pthread_t* next;
  pthread_t* current;
} pthread_pool_t;

int run_pthread(void* thread_instance, void* arguments_to_pass, pthread_t* out_pthread);
int wait_pthread_finishes(pthread_t* thread);

int process_bgcommand(char** argv);
int process_fgcommand(char** argv, int argc);
int process_pipecommand(char** argv);
int process_pipelining(char** argv, int argc);

int execute_command(char** argv);

void thread_bgcomm_execute(void* arg);
void thread_pipe_execute(void* arg);
void thread_wait_child(void* arg);

#endif