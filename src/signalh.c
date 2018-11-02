#include "signalh.h"
#include "utils.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h>

void signal_setup()
{
  //signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGCHLD, (void *)sighandle_callback);
  signal(SIGTTOU, (void *)sighandle_callback);
  signal(SIGTTIN, (void *)sighandle_callback);
}

int kill_pid(pid_t pid)
{
  return send_signal(pid, SIGKILL);
}

int send_signal(pid_t pid, int signal)
{
  if (kill(pid, signal) != 0)
    return errno;
  else return 1;
}
 
// signal handler for parent(main) process
void sighandle_callback(int signal)
{
  switch (signal)
  {
    case SIGCHLD:
      //printf("zombie created\n");
      break;
    
    case SIGTTOU: // background process가 stdout을 쓰게 해달라고 신호 보냄
      printf("background want to use stdout\n");
      break;

    case SIGTTIN: // background process가 stdin을 쓰게 해달라고 신호 보냄
      printf("background want to use stdin\n");
      //send_signal()
      //kill(-1, SIGTSTP); // send stop signal to process

      break;

    default: // or send signal normally
      send_signal(getpid(), signal);
  }
}

void sighandler_bg(int pid, char** command, int child_status)
{
  if (WIFEXITED(child_status))
  {
    printf("[%d]  + %d done\t", 1, pid);
  }
  else if (WCOREDUMP(child_status))
  {
    printf("[%d]  + %d segmentation fault\t", 1, pid);
  }
  else if (WIFSTOPPED(child_status))
  {
    printf("[%d]  + %d suspended\t", 1, pid);
  }
  else if (WIFSIGNALED(child_status))
  {
    printf("[%d]  + %d signaled\t", 1, pid);
  }
  else printf("[%d]  + %d not_supported_signaled\t", 1, pid);

  int i = 0;

  while (command[i] != NULL)
    printf("%s ", command[i++]);
 
  printf("\n");
}