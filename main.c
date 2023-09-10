#include "headers.h"

char *home_directory;
char old_pwd[4096];
pid_t shell_pid;
char *long_fg_process;
int long_fg_process_strlen;
int long_fg_process_duration;
pid_t bg_proc_pids[MAX_BG_PROCESSES_TRACKED];
char *bg_proc_names[MAX_BG_PROCESSES_TRACKED];
int bg_processes_count;
char *latest_prompt_input;
char **latest_commands_list;
int num_latest_commands;

int main()
{
    // Keep accepting commands

    home_directory = getcwd(NULL, 0);
    old_pwd[0] = '\0';
    shell_pid = getpid();
    bg_processes_count = 0;
    for (int i = 0; i < MAX_BG_PROCESSES_TRACKED; i++)
    {
        bg_proc_pids[i] = -1;
        bg_proc_names[i] = NULL;
    }
    latest_prompt_input = NULL;
    latest_commands_list = NULL;
    num_latest_commands = 0;

    while (1)
    {
        // Print appropriate prompt with username, systemname and directory before accepting input

        prompt();
        char input[4096];
        fgets(input, 4096, stdin);

        latest_prompt_input = (char *)malloc(sizeof(char) * (strlen(input) + 1));
        strcpy(latest_prompt_input, input);

        // to handle bg processes that exited/terminated:
        for (int j = 0; j < MAX_BG_PROCESSES_TRACKED; j++)
        {
            if (bg_processes_count == 0) // no children left to trace
                break;
            handle_bg_process_exits();
        }

        // free up space in global list of latest command strings
        if (latest_commands_list != NULL)
        {
            for (int i = 0; i < num_latest_commands; i++)
                free(latest_commands_list[i]);
            free(latest_commands_list);
            latest_commands_list = NULL;
            num_latest_commands = 0;
        }

        int num_commands;
        shell_command_data_ptr *commands = parse_input(input, strlen(input), &num_commands, 1);

        if (commands != NULL)
        {
            for (int i = 0; i < num_commands; i++)
            {
                // establish redirections, if any, overwriting pipeline redirections
                handle_redirection_and_execute(commands[i]);
            }
        }
        store_commands();
    }

    free(home_directory);
}
