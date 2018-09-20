#ifndef COMMANDS_H_
#define COMMANDS_H_

typedef int (*command_t)(int, char**);
typedef void (*command_error_handler)(int);

struct command_entry
{
  const char* command_name;
  command_t command_func;
  command_error_handler err;
};

struct command_entry* fetch_command(const char* command_name);

int do_pwd(int argc, char** argv);
void err_pwd(int err_code);

int do_cd(int argc, char** argv);
void err_cd(int err_code);

#endif // COMMANDS_H_
