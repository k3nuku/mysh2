#ifndef SIGNALH_H_
#define SIGNALH_H_

void signal_setup();

void sighandler_bg(int pid, char** command, int child_status);

int is_zombie_exist();
void zombie_alert(int signal);
void zombie_watchdog();
void zombieproc_handler(int signal);


int send_signal(int pid, int signal);
int kill_pid(int pid);
void kill_backgrounds();

#endif