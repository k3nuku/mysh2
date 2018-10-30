#include "threading.h"
#include "signalh.h"
#include "parser.h"
#include "socketpair.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int run_pthread(void* thread_instance, void* arguments_to_pass, pthread_t* out_pthread)
{
  pthread_t thread;
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  
  if (pthread_create(&thread, &thread_attr, thread_instance, arguments_to_pass) > -1)
  {
    *out_pthread = thread;

    return 1;
  }
  else return 0;
}

int wait_pthread_finishes(pthread_t* thread)
{
  int ret;
  
  if (pthread_join(*thread, (void *)&ret) != 0)
    return -1;
  else return ret;
}

// returns created pthread id
int process_bgcommand(char** argv)
{
  pid_t* pid;
  int child_status;
  char** argvdupd;

  argvdup(argv, &argvdupd); // copy argv to another memory space

  // using forked process to waiting working pthread(client) (non-blocking, synchronous callback)
  pid = (pid_t *)malloc(sizeof(pid_t));

  // fork current process to wait background task
  pid[0] = fork();

  if (pid[0] == 0) // child (processing bgtask)
  {
    int* pair; // create socketpair made with unix socket

    if ((pair = create_unix_socketpair()))
    {
      pthread_t thread;

      // server reads stdout and print to file

      // argv에 client socket reference 저장
      void** c_data = (void **)calloc(2, sizeof(void *));
      c_data[0] = (void *)(&pair[2]); // clientside-client socket
      c_data[1] = (void *)argvdupd;

      // run background process with pthread
      // and redirects output to argv[0].pool_id.stdout in current directory
      if (run_pthread((void *)thread_bgcomm_execute, (void *)c_data, &thread) > -1)
      {
        wait_pthread_finishes(&thread);
        // child status? -> print it

        //free(c_data[0]);
        //free(c_data[1]);
        //free(c_data);        
      }
      else fprintf(stderr, "failed to create pthread\n");
    }
    else fprintf(stderr, "failed to create socketpair\n");

    free_argv(argvdupd); // we need to free argvdupd twice because fork copies all of process memory

    exit(0);
  }
  else if (pid[0] > 0) // parent
  {
    pthread_t thread;

    void** c_data = (void **)calloc(2, sizeof(void *));
    c_data[0] = (void *)pid;
    c_data[1] = (void *)argvdupd;

    printf("[%d] %d\n", 1, pid[0]); // print pid and do work normally
    run_pthread((void *)thread_wait_child, (void *)c_data, &thread); // not to make a zombie proc
  }
  else
  {
    fprintf(stderr, "an unexpected error has occured while fork current process\n");
    return 0;
  }

  return 1;
}

int process_pipecommand(char** argv)
{
  // waiting to working pthread (blocking)
  pthread_t thread;
  pthread_attr_t thread_attr;

  pthread_attr_init(&thread_attr);
  pthread_create(&thread, &thread_attr, (void *)thread_bgcomm_execute, (void *)argv);

  return 0;
}

int process_fgcommand(char** argv, int argc)
{
  if (parse_has_pipelining(argv, argc))
  {
    if (!process_pipelining(argv, argc)) // pass it to pipelining processor
      return 0;
    else return 1;
  }
  else
  {
    if (!execute_command(argv)) // ordinary process it in foreground
      return 0;
    else return 1;  
  }
}

int process_pipelining(char** argv, int argc)
{
  // how to process n-th pipe
  // using unix socket, produces server-client model each pipes
  // parse pipe delimiter(|) and pop commands from beginning of the argv on
  // each thread redirects stdout to socket, then close the connection

  int head = 0;

  while (argv[head] != NULL)
  {
    int move = head;
    int count = 0;

    char** argv_pipe = NULL;

    while (strcmp(argv[move], "|") != 0)
    {
      char* data = argv[head];

      add_string_to_array(count, &argv_pipe, data);
      
      move++;
    }

    // argv_pipe;
    // using process_bgcommand to process 
    
    head = move + 1;
  }

  return -1;
}

int execute_command(char** argv)
{
  int child_status;
  int retval = 1;

  pid_t _pid;
  _pid = fork();

  switch (_pid)
  {
    case 0:
      execvp(argv[0], argv);
      retval = 0;
      break;

    default:
      if (_pid > 0)
        waitpid(-1, &child_status, 0);
      else
      {
        printf("fork failed\n");
        retval = 0;
      }
  }

  return retval;
}

void thread_bgcomm_execute(void* arg)
{
  // how about case of 'ls | grep &'
  // : each commands are being executed asynchronously, grep with "&" and ls
  // zsh does not execute pipelining; don't need to implement

  // deserialize arg -> clientside_socket, argv
  void** args = (void **)arg;
  int* c_fd = (int*)args[0]; // redirecting stdout to socket
  char** argv = (char**)args[1];

  // executing command
  execute_command(argv);
}

void thread_pipe_execute(void* arg)
{
  // setup unix socket for pipelining

}

void thread_wait_child(void* arg)
{
  void** c_data = (void **)arg;

  pid_t* pid = (pid_t *)c_data[0];
  char** argv = (char **)c_data[1];

  int child_status;

  waitpid(-1, &child_status, 0);
  sighandler_bg(*(int *)pid, argv, child_status);

  free((pid_t *)pid); 
  free_argv(argv);
  free(c_data);
}