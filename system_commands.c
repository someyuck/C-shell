#include "headers.h"

void handle_sigchld(int sig)
{
    printf("process exited.\n");
}

void system_command(shell_command_data_ptr command_data_ptr)
{
    // set handler for SIGCHLD
    signal(SIGCHLD, handle_sigchld);

    char **args = (char **)malloc(sizeof(char *) * (command_data_ptr->num_args + 1));
    for (int i = 0; i < command_data_ptr->num_args; i++)
    {
        args[i] = (char *)malloc(sizeof(char) * (strlen(command_data_ptr->words[i]) + 1));
        strcpy(args[i], command_data_ptr->words[i]);
    }
    args[command_data_ptr->num_args] = NULL;

    int ret = 0;
    pid_t child_pid = fork();
    if (child_pid < 0)
    {
        printf("failed fork\n");
        return;
    }

    else if (child_pid == 0)
    {
        ret = execvp(args[0], args);
        if (ret == -1)
        {
            printf("execvp: errno(%d) : %s\n", errno, strerror(errno));
        }
        else{
            if(command_data_ptr->fg_or_bg == 1)
            {
                setpgid(0, 0); // make gid different from parent, i.e. child is now a bg process
            }
            exit(0);
        }
    }
    else if (child_pid > 0)
    {
        if (command_data_ptr->fg_or_bg == 0) // foreground process
        {
            int status;
            time_t start_time = time(NULL);

            int wait_ret = waitpid(child_pid, &status, WUNTRACED);
            time_t child_time = time(NULL);
            child_time -= start_time;

            if (wait_ret == -1)
            {
                printf("waitpid error\n");
            }
            else
            {
                // if (WIFEXITED(status) == 0)
                // {
                //     printf("process did not exit normally\n");
                // }
                // else if (WIFEXITED(status) == 1)
                // {
                //     printf("process exited normally\n");
                // }

                if (child_time > 2)
                {
                    long_fg_process = (char *)malloc(sizeof(char) * (strlen(args[0]) + 3 + 1)); // "<proc> : ", duration directly printed in prompt.c
                    strcpy(long_fg_process, args[0]);
                    strcat(long_fg_process, " : ");
                    long_fg_process_strlen = strlen(long_fg_process);
                    long_fg_process_duration = child_time;
                }
            }
        }
        else if (command_data_ptr->fg_or_bg == 1) // background process
        {
            printf("%d\n", child_pid);
            // bg_processes = (pid_t*)malloc(sizeof(pid_t)*(bg_processes_count+ 1));
            // bg_processes[bg_processes_count] = child_pid;
            // bg_processes_count ++;
            
            // now waitpid() right before printing prompt in prompt.c, and check if each bg process has stopped or not (if so remove from list)
        }
    }
}
