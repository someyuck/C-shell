#include "headers.h"

void ping(char **args, int num_args)
{
    if (num_args == 3)
    {

        int target_pid = strtol(args[1], NULL, 0);
        if (errno == EINVAL || target_pid < 0)
        {
            fprintf(stderr, "\033[1;31mERROR: ping: invalid process ID\033[0m\n");
            return;
        }

        int target_signal = strtol(args[2], NULL, 0);
        if (errno == EINVAL)
        {
            fprintf(stderr, "\033[1;31mERROR: ping: invalid signal number\033[0m\n");
            return;
        }

        target_signal %= 32;

        int ret = kill(target_pid, target_signal);
        if (ret == -1)
            fprintf(stderr, "\033[1;31mERROR: kill: errno(%d) : %s\033[0m\n", errno, strerror(errno));
        else
            printf("sent signal %d to process with pid %d\n", target_signal, target_pid);
    }
    else
        fprintf(stderr, "\033[1;31mERROR: ping: incorrect syntax\033[0m\n");
}

void assign_signal_handlers(struct sigaction sigactionC, struct sigaction sigactionZ)
{
    sigactionC.sa_handler = handle_CTRL_C;
    sigactionZ.sa_handler = handle_CTRL_Z;

    sigactionC.sa_flags = SA_RESTART;
    sigactionZ.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sigactionC, NULL);
    sigaction(SIGTSTP, &sigactionZ, NULL);
}

void handle_CTRL_C(int signal)
{
    if (signal == SIGINT)
    {
        if (cur_fg_child_pid == -1)
            printf("\nNo foreground process is running at the moment. You can still enter commands (or just press enter)\n");
        else
        {
            // int ret = kill(cur_fg_child_pid, SIGINT);
            // if (ret == -1)
            //     fprintf(stderr, "\033[1;31mERROR: kill: errno(%d) : %s\033[0m\n", errno, strerror(errno));
            // else
            // {
            // printf("\nsent signal %d to process with pid %d\n", SIGINT, cur_fg_child_pid);
            cur_fg_child_pid = -1;
            free(cur_fg_child_pname);
            cur_fg_child_pname = NULL;
            // }
        }
    }
}

void handle_CTRL_Z(int signal)
{
    if (signal == SIGTSTP)
    {
        if (cur_fg_child_pid == -1)
            printf("\nNo foreground process is running at the moment. You can still enter commands (or just press enter)\n");
        else
        {
            if (setpgid(0, 0) == -1) // make gid different, hence making it a bg process
            {
                fprintf(stderr, "\033[1;31mERROR: setpgid: errno :%d: %s\033[0m\n", errno, strerror(errno));
                return;
            }

            // int ret = kill(cur_fg_child_pid, SIGTSTP); // send stop signal
            // if (ret == -1)
            //     fprintf(stderr, "\033[1;31mERROR: kill: errno(%d) : %s\033[0m\n", errno, strerror(errno));
            // else
            // printf("\nsent signal %d to process with pid %d\n", SIGTSTP, cur_fg_child_pid);

            // add to global bg processes list
            if (bg_processes_count < MAX_BG_PROCESSES_TRACKED)
            {
                for (int i = 0; i < MAX_BG_PROCESSES_TRACKED; i++)
                {
                    if (bg_proc_pids[i] == -1)
                    {
                        bg_proc_pids[i] = cur_fg_child_pid;
                        bg_proc_names[i] = (char *)malloc(sizeof(char) * (strlen(cur_fg_child_pname) + 1));
                        strcpy(bg_proc_names[i], cur_fg_child_pname);
                        free(cur_fg_child_pname);
                        cur_fg_child_pname = NULL;
                        bg_processes_count++;
                        break;
                    }
                }
            }
        }
    }
}

void handle_CTRL_D()
{
    // kill bg processes
    if (bg_processes_count < MAX_BG_PROCESSES_TRACKED)
    {
        for (int i = 0; i < MAX_BG_PROCESSES_TRACKED; i++)
        {
            if (bg_proc_pids[i] != -1)
            {
                while (kill(bg_proc_pids[i], SIGKILL) == -1)
                {
                    // kill till the child is dead
                }
                free(cur_fg_child_pname);
                cur_fg_child_pname = NULL;
                bg_processes_count--;
            }
        }
    }
    // kill the current fg process if running
    if (cur_fg_child_pid != -1)
    {
        while (kill(cur_fg_child_pid, SIGKILL) == -1)
        {
            // kill till the child is dead
        }
        cur_fg_child_pid = -1;
        free(cur_fg_child_pname);
        cur_fg_child_pname = NULL;
    }
    printf("\n\033[1;31mkilled all spawned processes\033[0m\n");
    printf("\033[1;31mlogging out of shell\033[0m\n");
    // exit(0);
}