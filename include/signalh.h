#ifndef SIGNALH_H_
#define SIGNALH_H_

void signal_setup();
void catch_sigint();
void catch_sigstp();
void sighandle_callback(int signal);

#endif