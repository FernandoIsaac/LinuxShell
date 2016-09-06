#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#define chdir _chdir

#else
#include <unistd.h>
#endif

#define MAX_LENGTH 1024
#define DELIMS " \t\r\n"

int main(int argc, char *argv[]) {
  char *cmd;
  char line[MAX_LENGTH];

  while (1) {
    printf("$ ");
    if (!fgets(line, MAX_LENGTH, stdin)) break;

    // Parse and execute command
    if ((cmd = strtok(line, DELIMS))) {
      // Clear errors
      errno = 0;

      if (strcmp(cmd, "cd") == 0) {
        char *arg = strtok(0, DELIMS);

        if (!arg) fprintf(stderr, "cd missing argument.\n");
        else chdir(arg);

      } else if (strcmp(cmd, "exit") == 0) {
        break;

      } else if (strcmp(cmd, "mkdir") == 0){
	char* arg = strtok(0, DELIMS);
	if (!arg) fprintf(stderr, "mkdir missing argument.\n");
	else mkdir(arg, 0700);
      } else if (strcmp(cmd, "rmdir") == 0){
	char* arg = strtok(0, DELIMS);
	if(!arg) fprintf(stderr, "rmdir missing argument.\n");
	else rmdir(arg);
      }else system(line);
      if (errno) perror("Command failed");
    }
  }

  return 0;
}
