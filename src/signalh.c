#include "signalh.h"
#include "utils.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h>

extern pid_t running_child_pid;

void signal_setup()
{
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTERM, (void *)zombie_alert);

  //zombie_watchdog();
}

void zombie_alert(int signal)
{
  int zombie_status = waitpid(-1, NULL, WNOHANG) > -1 ? 1 : 0;
  zombie_status = 1;

  if (zombie_status)
    printf("zombie alert\n");
}

void zombie_watchdog()
{
  struct sigaction sigact;
  sigact.sa_handler = zombieproc_handler;
  sigact.sa_flags = SA_NODEFER | SA_NOCLDWAIT;
  
  sigemptyset(&sigact.sa_mask);
  
  sigaction(SIGCHLD, &sigact, NULL);
}

int kill_pid(int pid)
{
  return send_signal(pid, SIGKILL);
}

void kill_backgrounds()
{
  if (running_child_pid > 0)
  {
    kill(running_child_pid, SIGKILL);
    printf("killed background process id: %d\n", running_child_pid);
  }
}

int send_signal(int pid, int signal)
{
  return kill(pid, signal) != 0 ? errno : 1;
}
 
void zombieproc_handler(int signal)
{
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

int is_zombie_exist()
{
  return waitpid(-1, NULL, WNOHANG) > 0 ? 1 : 0;
}

void sighandler_bg(int pid, char** command, int child_status)
{
  if (WIFEXITED(child_status))
    printf("[%d]  + %d done\t", 1, pid);
  else if (WCOREDUMP(child_status))
    printf("[%d]  + %d segmentation fault\t", 1, pid);
  else if (WIFSTOPPED(child_status))
    printf("[%d]  + %d suspended\t", 1, pid);
  else if (WIFSIGNALED(child_status))
  {
    switch (child_status)
    {
      case SIGKILL:
        printf("[%d]  + %d killed\t", 1, pid);
        break;

      case SIGINT:
        printf("[%d]  + %d interrupted\t", 1, pid);
        break;

      default:
        printf("[%d]  + %d signaled: %d\t", 1, pid, child_status);
    }
  }
  else
    printf("[%d]  + %d not_supported_signaled\t", 1, pid);

  if (command != NULL)
  {
    int i = 0;

    while (command[i] != NULL)
      printf("%s ", command[i++]);
  }

  printf("\n");
}