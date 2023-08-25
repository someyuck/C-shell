#include "headers.h"

char *home_directory;
char old_pwd[4096];
pid_t shell_pid;
char *long_fg_process;
int long_fg_process_strlen;
int long_fg_process_duration;
pid_t *bg_processes;
int bg_processes_count;

int main()
{
    // Keep accepting commands

    home_directory = getcwd(NULL, 0);
    old_pwd[0] = '\0';
    shell_pid = getpid();
    printf("shell pid:%d\n", shell_pid);

    while (1)
    {
        // Print appropriate prompt with username, systemname and directory before accepting input
        prompt();
        char input[4096];
        fgets(input, 4096, stdin);
        int num_commands;
        shell_command_data_ptr *commands = parse_input(input, strlen(input), &num_commands);

        for (int i = 0; i < num_commands; i++)
        {
            if (commands[i] == NULL || commands[i]->num_args == 0)
            {
                continue;
            }
            else if (strcmp(commands[i]->words[0], "warp") == 0)
            {
                warp(commands[i]->words, commands[i]->num_args);
            }
            else if (strcmp(commands[i]->words[0], "peek") == 0)
            {
                peek(commands[i]->words, commands[i]->num_args);
            }
            else if (strcmp(commands[i]->words[0], "proclore") == 0)
            {
                proclore(commands[i]->words, commands[i]->num_args);
            }
            else
            {
                system_command(commands[i]);
            }
        }
    }

    free(home_directory);
}
