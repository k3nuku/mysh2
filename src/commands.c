/**********************************************************************
 * Copyright (C) Jaewon Choi <jaewon.james.choi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 *********************************************************************/
#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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
  struct command_entry* ret_entry;

  ret_entry = (struct command_entry*)malloc(sizeof(struct command_entry*));
  ret_entry->command_name = command_name;
  
  if (strcmp(ret_entry->command_name, "pwd") == 0)
  {
    ret_entry->command_func = &do_pwd;
    ret_entry->err = &err_pwd;
  }
  else if (strcmp(command_name, "cd") == 0)
  {
    ret_entry->command_func = &do_cd;
    ret_entry->err = &err_cd;
  }
  else return NULL;

  return ret_entry;
}

int do_pwd(int argc, char** argv)
{
  char path[1024];

  if (getcwd(path, 1024) == NULL)
    return -1;
  else printf("%s\n", path);

  return 0;
}

void err_pwd(int err_code)
{
  printf("error occured while printing working directory\n");
}

int do_cd(int argc, char** argv)
{
  if (chdir(argv[1]) == -1)
    return -1;
  else return 0;
}

void err_cd(int err_code)
{
  printf("error occured while changing directory\n");
}
