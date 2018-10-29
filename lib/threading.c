#include "threading.h"

#include <pthread.h>
#include <unistd.h>
#include <errno.h>

// returns created pthread id
int process_bgcommand()
{


  //pthread_create();

  return 0;
}

int process_pipelining()
{
  return -1;
}

void thread_bgcomm_execute(void* arg)
{
  // how about case of 'ls | grep &'
  // : each command being executed asynchronously, grep with "&" and ls
  // zsh does not execute pipelining; not need to implementation


}

void thread_pipe_execute(void* arg)
{
  // setup unix socket for pipelining

}