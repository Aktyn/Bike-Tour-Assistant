#include "utils.h"
#include "Debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <sys/stat.h> // Include this header for R_OK

#define MKDIR_COMMAND_MAX (PATH_MAX + 6)

void extendBuffer(char **buffer, uint32_t new_size)
{
  char *new_buffer = (char *)realloc(*buffer, new_size * sizeof(char));
  if (new_buffer == NULL)
  {
    fprintf(stderr, "Failed to reallocate memory.\n");
    free(*buffer);
    return;
  }
  *buffer = new_buffer;
}

char *executeCommand(const char *command)
{
  DEBUG("Executing command: %s\n", command);

  FILE *pipe = popen(command, "r");
  if (!pipe)
  {
    perror("popen");
    return NULL;
  }

  char *cmd_output = (char *)malloc(0 * sizeof(char));
  uint32_t cmd_output_size = 0;

  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != NULL)
  {
    cmd_output_size += strlen(buffer) + 1;
    extendBuffer(&cmd_output, cmd_output_size);
    strcat(cmd_output, buffer);
  }

  int exit_status = pclose(pipe);
  if (WEXITSTATUS(exit_status) != 0)
  {
    fprintf(stderr, "Command exited with status: %d\n", WEXITSTATUS(exit_status));
    return NULL;
  }

  return cmd_output;
}

int safeCreateDirectory(const char *path)
{
  if (access(path, R_OK) != 0)
  {
    DEBUG("Creating directory: %s\n", path);

    char mkdir_command_input[MKDIR_COMMAND_MAX];
    snprintf(mkdir_command_input, MKDIR_COMMAND_MAX, "mkdir %s", path);
    mkdir_command_input[MKDIR_COMMAND_MAX - 1] = '\0'; // Ensure null termination
    char *mkdir_command = executeCommand(mkdir_command_input);
    if (mkdir_command == NULL)
    {
      printf("Error creating directory for camera photos\n");
      return 1;
    }
    else
    {
      free(mkdir_command);
      DEBUG("Directory created: %s\n", path);
    }
  }

  return 0;
}