#ifndef SIGNALH_H_
#define SIGNALH_H_

#include <sys/types.h>
#include <pthread.h>

void signal_setup();

void catch_sigint();
void catch_sigstp();
void catch_sigchld();

void sighandle_callback(int signal);
void sighandler_bg(int pid, char** command, int child_status);
int send_signal(pid_t pid, int signal);
int kill_pid(pid_t pid);

#endif