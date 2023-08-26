#include "headers.h"

void system_command(shell_command_data_ptr command_data_ptr)
{
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
            if (errno == ENOENT)
                fprintf(stderr, "\033[1;31mERROR: '%s' is not a valid command\033[0m\n", args[0]);
            else
                fprintf(stderr, "\033[1;31mexecvp: errno(%d) : %s\033[0m\n", errno, strerror(errno));

            // free up space
            for (int i = 0; i < command_data_ptr->num_args; i++)
            {
                free(args[i]);
            }

            free(args);
            exit(1);
        }
        else
        {
            if (command_data_ptr->fg_or_bg == 1)
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

            if (bg_processes_count < MAX_BG_PROCESSES_TRACKED)
            {
                for (int i = 0; i < MAX_BG_PROCESSES_TRACKED; i++)
                {
                    if (bg_proc_pids[i] == -1)
                    {
                        bg_proc_pids[i] = child_pid;
                        bg_proc_names[i] = (char *)malloc(sizeof(char) * (strlen(args[0]) + 1));
                        strcpy(bg_proc_names[i], args[0]);
                        bg_processes_count++;
                        break;
                    }
                }
            }
        }

        // free up space
        for (int i = 0; i < command_data_ptr->num_args; i++)
        {
            free(args[i]);
        }
    }
}

void handle_bg_process_exits()
{
    if (bg_processes_count == 0)
        return;
    int status;
    int pid = waitpid(-1, &status, WNOHANG);
    if (pid == -1)
    {
        ;
        // printf("waitpid errno(%d): %s\n", errno, strerror(errno));
    }
    else if (pid > 0)
    {
        for (int i = 0; i < MAX_BG_PROCESSES_TRACKED; i++)
        {
            if (bg_proc_pids[i] == pid)
            {
                if (WIFEXITED(status) == 0)
                {
                    fprintf(stderr, "\033[1;31m%s did not exit normally (%d)\033[0m\n", bg_proc_names[i], bg_proc_pids[i]);
                }
                else
                {
                    printf("\033[1;32m%s exited normally(%d)\033[0m\n", bg_proc_names[i], bg_proc_pids[i]);
                }

                bg_proc_pids[i] = -1;
                free(bg_proc_names[i]);
                break;
            }
        }
    }
}