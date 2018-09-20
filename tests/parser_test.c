#include "parser.h"
#include "utils.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

int main()
{
  int argc = -1;
  char** argv = NULL;

  parse_command("pwd", &argc, &argv);
  assert(argc == 1);
  assert(strncmp(argv[0], "pwd", strlen("pwd")) == 0);
  assert(argv[1] == NULL);
  FREE_2D_ARRAY(argc, argv);
  
  parse_command("pwd ", &argc, &argv);
  assert(argc == 1);
  assert(strncmp(argv[0], "pwd", strlen("pwd")) == 0);
  assert(argv[1] == NULL);
  FREE_2D_ARRAY(argc, argv);

  parse_command("     cd tests", &argc, &argv);
  assert(argc == 2);
  assert(strncmp(argv[0], "cd", strlen("cd")) == 0);
  assert(strncmp(argv[1], "tests", strlen("tests")) == 0);
  assert(argv[2] == NULL);
  FREE_2D_ARRAY(argc, argv);

  parse_command("cat some.txt > \"what I am writing\"", &argc, &argv);
  assert(argc == 4);
  assert(strncmp(argv[0], "cat", strlen("cat")) == 0);
  assert(strncmp(argv[1], "some.txt", strlen("some.txt")) == 0);
  assert(strncmp(argv[2], ">", strlen(">")) == 0);
  assert(strncmp(argv[3], "what I am writing", strlen("what I am writing")) == 0);
  assert(argv[4] == NULL);
  FREE_2D_ARRAY(argc, argv);

  return 0;
}
