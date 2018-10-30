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
  catch_sigint();
  catch_sigstp();
}

// ctrl+c
void catch_sigint()
{
  signal(SIGINT, (void *)sighandle_callback);
}

// ctrl+z
void catch_sigstp()
{
  signal(SIGTSTP, (void *)sighandle_callback);
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
    case SIGINT: // ignoring ctrl+c
      break;

    case SIGTSTP: // ignoring ctrl+z
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
    printf("[%d]  + %d core dumped\t", 1, pid);
  }
  else if (WIFSTOPPED(child_status))
  {
    printf("[%d]  + %d suspended\t", 1, pid);
  }
  else if (WIFSIGNALED(child_status))
  {
    if (child_status == 11)
      printf("[%d]  + %d Segmentation fault\t", 1, pid);
    else printf("[%d]  + %d signaled\t", 1, pid);
  }
  else printf("[%d]  + %d not_supported_signaled\t", 1, pid);

  int i = 0;

  while (command[i] != NULL)
    printf("%s ", command[i++]);
 
  printf("\n");
}