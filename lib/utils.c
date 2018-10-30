#include "utils.h"

#include <string.h>

void argvdup(char** argv, char*** out_argv)
{
  int count = 0;

  while (argv[count] != NULL)
    count++;

  char** argvd = (char**)calloc(count, sizeof(char*));

  for (int i = 0; i < count; i++)
  {
    argvd[i] = strdup(argv[i]);

    if (i == count - 1)
      argvd[i + 1] = NULL;
  }

  *out_argv = argvd;
}

void free_argv(char** argv)
{
  int count = 0;

  for (int i = 0; argv[i] != NULL; i++)
    count++;

  FREE_2D_ARRAY(count, argv);
}