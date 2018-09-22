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
 *  Argument    : [darray] as 2-stack pointer of char (string array)
                  [string] as string
 *  Return      : length of string array as int
 *                + affected string array as [darray] in input parameter
 */
int add_string_to_array(int count_of_length, char*** darray, char* string)
{
  char** array = *darray;

  printf("---\nstg2: add_string_to_array input array address: %p\n", array);
  printf("input string: %s, address: %p\n", string, string);

  if (!*darray)
  {
    printf("init string array\n");
    array = (char**)calloc(1, sizeof(char*));
    array[0] = NULL; // init first row to NULL
  }

  array = (char**)realloc(array, (count_of_length + 2) * sizeof(char*)); // Add 1 more space
  printf("init completed, realloc size of array, address: %p\n", array);

  array[count_of_length] = (char*)calloc(strlen(string) + 1, sizeof(char*)); // calloc
  printf("created string pointer, string length: %d, array length: %d\n", strlen(string), strlen(string) + 1);

  strcpy(array[count_of_length], string);
  printf("wrote %dth element with \"%s\", address: %p\n", count_of_length, array[count_of_length], array[count_of_length]);

  array[count_of_length + 1] = NULL; // add NULL at last of element in extended area
  printf("wrote last element(%dth) with %s, address: %p\n", count_of_length + 1, NULL, array[count_of_length + 1]);

  *darray = array;

  return ++count_of_length;
}

void parse_command(const char* input,
                   int* argc, char*** argv)
{
  // tokenizing + (determining if token has double quote, parse) -> push to argv
  char* token;
  char** temp_argv = NULL;
  int temp_argc = 0;

  token = (char*)calloc(strlen(input) + 1, sizeof(char*));
  strcpy(token, input); // const char* -> char* conversion
  
  token = strtok(token, " ");
 
  while (token != NULL)
  {
    printf("---\nstg1: token: %s, address: %p\n", token, token);
    printf("first letter: %c\n", token[0]);

    if (token[0] == '\"') // double quoted token
    {
      printf("found double quote, address: %p\n", token);

      int found = 0;
      char* temp;
      char temp_dquoted[1024];

      strcat(temp_dquoted, token + 1);
      strcat(temp_dquoted, " ");

      if (!(temp = strtok(NULL, "\"")))
      {
        temp_argc = -1;

        break;
      }

      strcat(temp_dquoted, temp);

      printf("assembled double quoted string: %s\n", temp_dquoted);

      printf("before add, dquoted_string: %s\n", temp_dquoted);
      temp_argc = add_string_to_array(temp_argc, &temp_argv, temp_dquoted);
    }
    else
    {
      printf("before add, token: %s\n", token);
      temp_argc = add_string_to_array(temp_argc, &temp_argv, token);
    }

    printf("add completed, current string array length: %d\n", temp_argc);
    token = strtok(NULL, " ");
  }

  if (temp_argc > -1) // 정상이면
  {
    free(token);
  
    printf("referencing temporary argv: 1stelem: %s, address: %p\n", temp_argv[0], temp_argv);
    *argv = temp_argv;
  }
  else free(argv); // 에러나면

  printf("referencing temporary argc: %d to argument, address %p\n", temp_argc, &temp_argc);
  *argc = temp_argc;
}