#include "signalh.h"

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

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

int kill(pid_t pid, int signal)
{
  return kill(pid, signal) > 0 ? 1 : 0;
}

void sighandle_callback(int signal)
{
  switch (signal)
  {
    case SIGINT:
      printf("got sigint\n");
      break;

    case SIGTSTP:
      // [1]  + 10910 suspended  wget https://git.kernel.org/torvalds/t/linux-4.19-rc7.tar.gz
      printf("got sigstp\n");
      break;
    
    default:
      printf("other signal received\n");
  }
}