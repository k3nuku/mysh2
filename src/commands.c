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
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

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
  },
  {
    "fg",
    do_fg,
    err_fg
  },
  {
    "kill",
    do_kill,
    err_kill
  }
};

struct command_entry* fetch_command(const char* command_name)
{
  struct command_entry* ret_entry;

  if (strcmp(command_name, "pwd") == 0)
    ret_entry = &commands[0];
  else if (strcmp(command_name, "cd") == 0)
    ret_entry = &commands[1];
  else return NULL;
  
  return ret_entry;
}

int do_pwd(int argc, char** argv)
{
  errno = 0;

  char path[MAX_DIR_LENGTH];

  if (getcwd(path, MAX_DIR_LENGTH))
    printf("%s\n", path);

  return errno ? errno : 0;
}

void err_pwd(int err_code)
{
  switch (err_code)
  {
    case EACCES:
      printf("pwd: you don't have permission to display current directory\n");
      break;

    case EFAULT:
      printf("pwd: \n");
      break;

    case EINVAL:
      printf("pwd: \n");
      break;

    default:
      printf("pwd: an unknown error occured while printing current working directory\n");
      break;
  }
}

int do_cd(int argc, char** argv)
{
  errno = 0;

  if (argv[1])
  {
    chdir(argv[1]);
    
    return errno ? errno : 0;
  }
  else return -1025;
}

void err_cd(int err_code)
{
  switch (err_code)
  {
    case -1025:
      fprintf(stderr, "cd: directory should be served as argument\n");
      break;

    case ENOTDIR:
      fprintf(stderr, "cd: not a directory\n");
      break;

    case ENOENT:
      fprintf(stderr, "cd: no such file or directory\n");
      break;

    default:
      fprintf(stderr, "cd: an error occured while changing directory\n");
      break;
  }
}

int do_fg(int argc, char** argv)
{
  return -1;
}

void err_fg(int err_code)
{
  
}

int do_kill(int argc, char** argv)
{
  return -1;
}

void err_kill(int err_code)
{
  
}
