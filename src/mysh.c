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
#include "utils.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
  signal_setup();

  char command_buffer[4096] = { 0, };
  int child_status;

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
        comm_entry->err(ret);
      }
    } else if (does_exefile_exists(argv[0])) { // check whether it is executable binary then execute
      // check whether program should be ran over background
      if (parse_is_background(argv)) // background process
      {
        // processing background task with pthread library
        printf("bgtest success");


      }
      else // normal foreground process
      {
        pid_t _pid;
        _pid = fork();

        switch (_pid)
        {
          case 0:
            execvp(argv[0], argv);
            
            printf("something went wrong. child is going exit.\n");
            exit(0);

            break;

          default:
            if (_pid > 0)
              waitpid(-1, &child_status, 0);
            else printf("fork failed\n");

            break;
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
