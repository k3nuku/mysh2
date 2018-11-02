#ifndef SIGNALH_H_
#define SIGNALH_H_

#include <sys/types.h>
#include <pthread.h>

void signal_setup();

void sighandler_bg(int pid, char** command, int child_status);

int is_zombie_exist();
void zombie_watchdog();
void zombieproc_handler(int signal);

int send_signal(pid_t pid, int signal);
int kill_pid(pid_t pid);
void kill_backgrounds();

#endif