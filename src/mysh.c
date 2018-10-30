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
#include "signalh.h"
#include "threading.h"
#include "utils.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main()
{
  signal_setup();

  char command_buffer[4096] = { 0, };

  while (fgets(command_buffer, 4096, stdin) != NULL) {
    int argc = -1;
    char** argv = NULL;

    if (command_buffer[0] == '\n')
      continue;

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
        comm_entry->err(ret);
      }
    } else if (does_exefile_exists(argv[0])) { // check whether it is executable binary then execute
      if (parse_is_background(argv, &argc)) // check whether program should be ran over background
      {
        if (!process_bgcommand(argv)) // processing background task with pthread library
          fprintf(stderr, "unexpected error has occured while executing background process.\n");
      }
      else // normal foreground process 
      {
        if (!process_fgcommand(argv, 0))
        {
          fprintf(stderr, "unexpected error has occured while executing command.\nmysh now exit.\n");

          return -1;
        }
      }
    } else {
      assert(comm_entry == NULL);
      fprintf(stderr, "%s: command not found.\n", argv[0]);
    }

    FREE_2D_ARRAY(argc, argv);
  }

  return 0;
}
