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
#include "parser.h"
#include "utils.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>
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

    if (comm_entry != NULL) { // pre-defined command in mysh
      int ret = comm_entry->command_func(argc, argv);
      if (ret != 0) {
        comm_entry->err(ret, argv);
      }
    } else if (does_exefile_exists(argv[0])) { // check whether it is executable binary then execute
      // TODO: Execute the program of argv[0].
      printf("exefile exists\n");

    } else {
      assert(comm_entry == NULL);
      fprintf(stderr, "%s: command not found.\n", argv[0]);
    }

    FREE_2D_ARRAY(argc, argv);
  }

  return 0;
}
