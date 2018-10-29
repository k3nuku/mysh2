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
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *  Function    : add_string_to_array
 *  Description : Add string into array. If array is not initialized, then initialize and add string.
 *  Argument    : [count_of_length] as integer
 *                [darray] as 2-stack pointer of char (string array)
 *                [string] as string
 *  Return      : Added row count. If succeeded then returns 1 as output.
 *                + affected string array as [darray] in input parameter
 */
int add_string_to_array(int count_of_length, char*** darray, char* string)
{
  char** array = *darray;

  if (!*darray)
  {
    array = (char**)calloc(1, sizeof(char*));
    array[0] = NULL;
  }

  array = (char**)realloc(array, (count_of_length + 2) * sizeof(char*));
  array[count_of_length] = (char*)calloc(strlen(string) + 1, sizeof(char));

  strcpy(array[count_of_length], string);

  array[count_of_length + 1] = NULL;
  *darray = array;

  return ++count_of_length;
}

void parse_command(const char* input,
                   int* argc, char*** argv)
{
  char** temp_argv = NULL;
  int temp_argc = 0;

  char* token = (char*)calloc(strlen(input) + 1, sizeof(char));
  strcpy(token, input);

  char* begin_of_the_token = token;

  token = strtok(token, " ");
 
  while (token != NULL)
  {
    if (token[0] == '\"')
    {
      char temp_dquoted[1024];

      strcat(temp_dquoted, token + 1);
      strcat(temp_dquoted, " ");

      if ((token = strtok(NULL, "\"")) == NULL)
      {
        temp_argc = -1;

        break;
      }

      strcat(temp_dquoted, token);
      temp_argc = add_string_to_array(temp_argc, &temp_argv, temp_dquoted);
    }
    else temp_argc = add_string_to_array(temp_argc, &temp_argv, token);

    token = strtok(NULL, " ");
  }

  free(begin_of_the_token);

  if (temp_argc > -1)
  {

    if (temp_argv[temp_argc - 1][strlen(temp_argv[temp_argc - 1]) - 1] == '\n')
      temp_argv[temp_argc - 1][strlen(temp_argv[temp_argc - 1]) - 1] = '\0';

    *argv = temp_argv;
  }

  *argc = temp_argc;
}

// wanna remove "&" string, provide argc to this function as parameter at function call
int parse_is_background(char** argv, int* argc)
{
  int is_bgcomm = 0;

  if (*argc > 0)
  {
    if (strcmp(argv[*argc - 1], "&") == 0)
    {
      is_bgcomm = 1;

      free(argv[*argc - 1]);

      argv[*argc - 1] = argv[*argc];
      *argc = *argc - 1;
    }
  }
  else
  {
    int argv_count = 0;

    while (argv[argv_count] != NULL)
      argv_count++;

    if (strcmp(argv[argv_count - 1], "&") == 0)
      is_bgcomm = 1;
  }

  return is_bgcomm;
}