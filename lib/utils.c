#include "utils.h"
#include "parser.h"

#include <string.h>

void argvdup(char** argv, char*** out_argv)
{
  int count = 0;

  while (argv[count] != NULL)
    count++;

  char** argvd = (char**)calloc(count, sizeof(char*));

  for (int i = 0; i < count; i++)
    add_string_to_array(i, &argvd, argv[i]);

  *out_argv = argvd;
}

void free_argv(char** argv)
{
  int count = 0;

  for (int i = 0; argv[i] != NULL; i++)
    count++;

  FREE_2D_ARRAY(count, argv);
}