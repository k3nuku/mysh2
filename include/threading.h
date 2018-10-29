#ifndef THREADING_H_
#define THREADING_H_

int process_bgcommand();
int process_pipelining();

void thread_bgcomm_execute(void* arg);
void thread_pipe_execute(void* arg);

#endif