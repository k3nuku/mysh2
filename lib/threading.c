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
        // put pthread into threadpool
        // but mysh implementation requires just one background process, no thread pool needed

        wait_pthread_finishes(&thread);
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

      // argv에 client socket reference 저장
      void** c_data = (void **)calloc(2, sizeof(void *));
      c_data[0] = (void *)(&pair[2]); // clientside-client socket
      c_data[1] = (void *)argvdupd;

      // run background process with pthread
      if (run_pthread((void *)thread_bgcomm_execute, (void *)c_data, &thread) > -1)
      {
        // put pthread into threadpool
        // but mysh implementation requires just one background process, no thread pool needed

        wait_pthread_finishes(&thread);
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
    if (!execute_command(argv, 0, NULL, NULL, -1)) // ordinary process it in foreground
      return 0;
    else return 1;  
  }
}

// how to process n-th pipe
// using unix socket, produces server-client model each pipes
// parse pipe delimiter(|) and pop commands from beginning of the argv on
// each thread redirects stdout to socket, then close the connection
int process_pipelining(char** argv, int argc)
{
  int *pair, *last_pair;
  int head = 0;

  //int back_stdout = dup(STDOUT_FILENO);

  while (argv[head] != NULL)
  {
    char** argv_pipe = NULL;
    head = parse_until_pipe_found(argv, &argv_pipe, head);

    // if there is a next command then create new pipe
    if (argv[head] != NULL)
    {
      if ((pair = create_unix_socketpair()) < 0)
      {
        fprintf(stderr, "failed to create socketpair for pipelining\n");
        return 0;
      }
    }

    // execute command at argv_pipe
    // redirect all stdout to provided client socket using dup2
    //  : ex> cmd1 | cmd2 | cmd3 | cmd4;
    //    parent(main)=close-stdout, redirect stdout to socket (stdout to child)
    //    child(n-th-pipe)=close-stdin, redirect stdin/stdout to socket (stdout to next child or main)
    //    printf("executing %s\n", argv_pipe[0]);
    execute_command(argv_pipe, 1, &last_pair, pair,
      argv[head] == NULL ? 1 : 0);

    free_argv(argv_pipe); // dispose argv used at individual process pipelining
  }

  //dup2(back_stdout, STDOUT_FILENO)

  return 1;
}

int execute_command(char** argv, int is_pipecomm, int** out_last_pair, int* pair, int last_command)
{  
  int child_status;
  int retval = 1;

  int* last_pair;

  pid_t _pid;
  _pid = fork();

  if (is_pipecomm)
    last_pair = *out_last_pair;

  switch (_pid)
  {
    case 0:
      if (is_pipecomm)
      {
        if (last_pair) // middle command, last command
        {
          dup2(last_pair[1], STDIN_FILENO);
          close(last_pair[1]);
          close(last_pair[2]);
        }
        else // first command
        {
          close(pair[1]);
          dup2(pair[2], STDOUT_FILENO);
          close(pair[2]);
        }

        if (!last_command) // first command, middle command
        {
          close(pair[1]);
          dup2(pair[2], STDOUT_FILENO);
          close(pair[2]);
        }
      }

      execvp(argv[0], argv);

      fprintf(stderr, "exec failed\n");
      retval = 0; // non-reachable code block
      break;

    default:
      if (_pid > 0)
      {
        if (is_pipecomm)
        {
          if (last_pair) // middle command, last command
          {
            close(last_pair[1]);
            close(last_pair[2]);
          }

          if (!last_command) // first command, middle command
            *out_last_pair = pair;
        }
        
        waitpid(-1, &child_status, 0);
      }
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
  int* c_fd = (int*)args[0]; // purpose of redirecting stdout to socket
  char** argv = (char**)args[1];

  // closing stdin/out
  close(0);
  close(1);
  
  if (dup2(*c_fd, 1) > 0)
  {
    execute_command(argv, 0, NULL, NULL, -1); // executing command
  }
  else fprintf(stderr, "an error has occured while redirect stdout to socket\n");
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