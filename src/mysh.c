#include "commands.h"
#include "parser.h"
#include "utils.h"
#include "fs.h"

#include <stdio.h>
#include <assert.h>

#include <errno.h>

int main()
{
  char command_buffer[4096] = { 0, };

  while (fgets(command_buffer, 4096, stdin) != NULL) {
    int argc = -1;
    char** argv = NULL;

    parse_command(command_buffer, &argc, &argv);

    assert(argv != NULL);
    if (strcmp(argv[0], "exit") == 0) {
      FREE_2D_ARRAY(argc, argv);
      break;
    }

    struct command_entry* comm_entry = fetch_command(argv[0]);

    if (comm_entry != NULL) {
      int ret = comm_entry->command_func(argc, argv);
      if (ret != 0) {
        comm_entry->err(ret);
      }
    } else if (does_exefile_exists(argv[0])) {
      // TODO: Execute the program of argv[0].
    } else {
      assert(comm_entry == NULL);
      fprintf(stderr, "%s: command not found.\n", argv[0]);
    }

    FREE_2D_ARRAY(argc, argv);
  }

  return 0;
}
