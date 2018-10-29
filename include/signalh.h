#ifndef SIGNALH_H_
#define SIGNALH_H_

#include <sys/types.h>

void signal_setup();
void catch_sigint();
void catch_sigstp();
void sighandle_callback(int signal);
int send_signal(pid_t pid, int signal);
int kill_pid(pid_t pid);

#endif