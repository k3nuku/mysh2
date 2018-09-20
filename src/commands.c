#include "commands.h"
#include <string.h>

static struct command_entry commands[] =
{
  {
    "pwd",
    do_pwd,
    err_pwd
  },
  {
    "cd",
    do_cd,
    err_cd
  }
};

struct command_entry* fetch_command(const char* command_name)
{
  // TODO: Fill it.

  return NULL;
}

int do_pwd(int argc, char** argv)
{
  // TODO: Fill it.

  return -1;
}

void err_pwd(int err_code)
{
  // TODO: Fill it.
}

int do_cd(int argc, char** argv)
{
  // TODO: Fill it.

  return -1;
}

void err_cd(int err_code)
{
  // TODO: Fill it.
}
