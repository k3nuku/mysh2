#ifndef THREADING_H_
#define THREADING_H_

#include <pthread.h>
#include <commands.h>

struct thread_argument
{
  int pid;
  char** argv;
} thread_argument;

int run_pthread(void* thread_instance, void* arguments_to_pass, pthread_t* out_pthread);
int kill_pthread(pthread_t thread);
int wait_pthread_finishes(pthread_t* thread);

int process_bgcommand(char** argv);
int process_fgcommand(char** argv, int argc);
int process_pipecommand(char** argv);
int process_pipelining(char** argv, int argc);
int process_internal_bgcommand(struct command_entry* entry, char** argv, int argc);

int execute_command(char** argv, int is_bgcomm, int is_pipecomm, int** last_pair, int* pair, int last_command);

void thread_bgcomm_execute(void* arg);
void thread_pipe_execute(void* arg);
void thread_wait_child(void* arg);

#endif