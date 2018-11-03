#include "threading.h"
#include "signalh.h"
#include "parser.h"
#include "socketpair.h"
#include "utils.h"
#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <signal.h>

#include <sys/wait.h>
#include <sys/types.h>

int running_child_pid;

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

int kill_pthread(pthread_t thread)
{
  int ret;

  pthread_kill(thread, SIGKILL);
  return pthread_join(thread, (void **)&ret) != 0 ? -1 : 0;
}

int wait_pthread_finishes(pthread_t* thread)
{
  int ret;

  if (pthread_join(*thread, (void **)&ret) != 0)
    return -1;
  else return ret;
}

// returns created pthread id
int process_bgcommand(char** argv)
{
  pid_t child_id;
  char** argvdupd;
  struct thread_argument* sa;

  if ((child_id = execute_command(argv, 1, 0, NULL, NULL, -1)) == -1) // executing background command, with other pgid
  {
    fprintf(stderr, "an error has occured while excuting background command\n");
    
    return 0;
  }
  else printf("[1] %d\n", child_id);

  argvdup(argv, &argvdupd);

  sa = (struct thread_argument*)malloc(sizeof(struct thread_argument));
  sa->pid = child_id;
  sa->argv = argvdupd;

  pthread_t thread;
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create(&thread, &thread_attr, (void *)thread_wait_child, (void *)sa) < 0)
  {
    fprintf(stderr, "failed to create pthread while processing background command\n");

    return 0;
  }
  else return 1;
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
    if (execute_command(argv, 0, 0, NULL, NULL, -1) == -1) // ordinary process it in foreground
      return 0;
    else return 1;  
  }
}

int process_internal_bgcommand(struct command_entry* entry, char** argv, int argc)
{
  struct thread_argument* sa;
  char** argvdupd;
  int child_status;
  int _pid;

  if ((_pid = fork()) == 0)
  {
    setpgid(0, 0); // 별도 process group 할당(stdin/out 분리), for background processing
    tcsetpgrp(STDIN_FILENO, getppid()); // 가져온 foreground의 stdin을 parent pid에 연결하여 되돌림

    int ret = entry->command_func(argc, argv);
    if (ret != 0)
      entry->err(ret);

    free_argv(argv);
    exit(0);
  }
  else if (_pid > 0)
  {
    printf("[1] %d\n", _pid);

    argvdup(argv, &argvdupd);

    sa = (struct thread_argument*)malloc(sizeof(struct thread_argument));
    sa->pid = _pid;
    sa->argv = argvdupd;

    pthread_t thread;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&thread, &thread_attr, (void *)thread_wait_child, (void *)sa) < 0)
    {
      fprintf(stderr, "failed to create pthread while processing background command\n");
      return 0;
    }
  }
  else
  {
    fprintf(stderr, "mysh: unexpected error while fork process for internal command\n");
    return 0; 
  }

  return 1;
}

// how to process n-th pipe
// using unix socket, produces server-client model each pipes
// parse pipe delimiter(|) and pop commands from beginning of the argv on
// each thread redirects stdout to socket, then close the connection
int process_pipelining(char** argv, int argc)
{
  int *pair, *last_pair = NULL;
  int head = 0;

  while (argv[head] != NULL)
  {
    char** argv_pipe = NULL;
    head = parse_until_pipe_found(argv, &argv_pipe, head);

    // if there is a next command then create a new pipe
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
    if (does_exefile_exists(argv_pipe[0]))
      execute_command(argv_pipe, 0, 1, &last_pair, pair,
        argv[head] == NULL ? 1 : 0); // ignoring returned pid
    else
      fprintf(stderr, "%s: command not found.\n", argv_pipe[0]);

    free_argv(argv_pipe); // dispose argv used at individual process pipelining
  }

  return 1;
}

int execute_command(char** argv, int is_bgcomm, int is_pipecomm, int** out_last_pair, int* pair, int last_command)
{  
  int child_status;
  int retval = 1;

  int* last_pair;

  if (is_pipecomm)
    last_pair = *out_last_pair;

  pid_t _pid;
  _pid = fork();

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

      if (is_bgcomm)
      {
        setpgid(0, 0); // 별도 process group 할당(stdin/out 분리), for background processing
        tcsetpgrp(STDIN_FILENO, getppid()); // 가져온 foreground의 stdin을 parent pid에 연결하여 되돌림
      }

      signal(SIGINT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);

      execvp(argv[0], argv);

      fprintf(stderr, "exec failed\n");
      retval = 0; // unreachable code block
      exit(0);
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
            
            free(last_pair);
          }

          if (!last_command) // first command, middle command
            *out_last_pair = pair;
        }

        if (!is_bgcomm)
          waitpid(_pid, &child_status, 0);
      }
      else
      {
        printf("fork failed\n");
        retval = 0;
      }
  }

  return retval ? _pid : -1;
}

void thread_wait_child(void* arg) // for background task
{
  struct thread_argument* ta = (struct thread_argument*)arg;

  running_child_pid = 0;
  running_child_pid = ta->pid;

  int child_status;

  waitpid(ta->pid, &child_status, 0);
  sighandler_bg(ta->pid, ta->argv, child_status);

  running_child_pid = 0;

  free_argv(ta->argv);
  free(ta);
}
