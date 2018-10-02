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
#include "fs.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int does_exefile_exists(const char* path)
{
  struct stat stat_res;
  int found = 0;

  if (!(found = check_file_executable(path)))
  {
    const char* system_path = getenv("PATH");
    char* path_token = (char*)calloc(strlen(system_path) + 1, sizeof(char));
    char* begin_of_path_token = path_token;

    path_token = strcpy(path_token, system_path);    
  	path_token = strtok(path_token, ":");

  	while (path_token != NULL)
  	{
      char* temp_pathres = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));

      strcat(temp_pathres, path_token);
      strcat(temp_pathres, "/");
      strcat(temp_pathres, path);

      if (check_file_executable(temp_pathres))
      {
        found = 1;
        free(temp_pathres);

        break;
      }

      free(temp_pathres);

  		path_token = strtok(NULL, ":");
  	}

    free(begin_of_path_token);
  }

  return found ? 1 : 0;
}

int check_file_executable(const char* path)
{
  struct stat stat_res;

  return (!stat(path, &stat_res) && !S_ISDIR(stat_res.st_mode) &&
    (stat_res.st_mode & S_IXUSR)) ? 1 : 0;
}