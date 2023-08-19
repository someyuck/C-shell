#include "headers.h"

char *home_directory;
char *old_pwd;


int main()
{
    // Keep accepting commands

    home_directory = getcwd(NULL, 0);
    old_pwd = NULL;

    while (1)
    {
        // Print appropriate prompt with username, systemname and directory before accepting input
        prompt();
        char input[4096];
        fgets(input, 4096, stdin);
        int num_commands;
        shell_command_data_ptr *commands = parse_input(input, strlen(input), &num_commands);
        if(strcmp(commands[0]->words[0], "warp") == 0)
        {
            warp(commands[0]->words, commands[0]->num_args);
        }
    }

    free(home_directory);
}
