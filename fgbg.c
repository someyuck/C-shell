#include "headers.h"

void fg(char **args, int num_args)
{
    if (num_args == 2)
    {
        pid_t target_pid = strtol(args[1], NULL, 10);
        if (errno == EINVAL || target_pid < 0)
        {
            fprintf(stderr, "\033[1;31mERROR: bg: invalid process ID\033[0m\n");
            return;
        }

        char *file_path = (char *)malloc(sizeof(char) * (strlen("/proc/") + strlen(args[1]) + strlen("/stat") + 1));
        strcpy(file_path, "/proc/");
        strcat(file_path, args[1]);
        strcat(file_path, "/stat");

        FILE *fp = fopen(file_path, "r");
        if (fp == NULL)
        {
            printf("\033[1;31mNo such process found\033[0m\n");
            free(file_path);
            return;
        }

        int pid;
        char pname[4096];
        pname[4095] = '\0';
        char state[2];
        state[1] = '\0';
        long process_group;
        long tpgid;
        char dummy[4096];

        fscanf(fp, "%d %s %s %s %ld %s %s %ld", &pid, pname, state, dummy, &process_group, dummy, dummy, &tpgid);
        fclose(fp);

        // if process is stopped make it run again
        if (state[0] == 'T')
        {
            if (kill(target_pid, SIGCONT) == -1)
                fprintf(stderr, "\033[1;31mERROR: kill: errno(%d) : %s\033[0m\n", errno, strerror(errno));
            else
                printf("process with pid %d running now\n", pid);
        }

        // now send it back to fg by setting its group id equal to current processes group id

        setpgid(pid, tpgid);

        // update global fg process tracker
        cur_fg_child_pid = pid;
        if (cur_fg_child_pname != NULL)
        {
            free(cur_fg_child_pname);
            cur_fg_child_pname = (char *)malloc(sizeof(char) * (strlen(pname) + 1));
            strcpy(cur_fg_child_pname, pname);
        }
        free(file_path);
    }
    else
        fprintf(stderr, "\033[1;mERROR: bg : invalid syntax\033[0m\n");
}

void bg(char **args, int num_args)
{
    if (num_args == 2)
    {
        pid_t target_pid = strtol(args[1], NULL, 10);
        if (errno == EINVAL || target_pid < 0)
        {
            fprintf(stderr, "\033[1;31mERROR: bg: invalid process ID\033[0m\n");
            return;
        }

        char *file_path = (char *)malloc(sizeof(char) * (strlen("/proc/") + strlen(args[1]) + strlen("/stat") + 1));
        strcpy(file_path, "/proc/");
        strcat(file_path, args[1]);
        strcat(file_path, "/stat");

        FILE *fp = fopen(file_path, "r");
        if (fp == NULL)
        {
            printf("\033[1;31mNo such process found\033[0m\n");
            free(file_path);
            return;
        }

        int pid;
        char pname[4096];
        pname[4095] = '\0';
        char state[2];
        state[1] = '\0';

        fscanf(fp, "%d %s %s", &pid, pname, state);
        fclose(fp);

        if (state[0] == 'R' || state[0] == 'S')
        {
            printf("process with pid %d already running in background\n", pid);
        }
        else if (state[0] == 'T')
        {
            if (kill(target_pid, SIGCONT) == -1)
            {
                fprintf(stderr, "\033[1;31mERROR: kill: errno(%d) : %s\033[0m\n", errno, strerror(errno));
            }
            else
                printf("process with pid %d running now\n", pid);
        }
        else
            printf("\033[1;31mNo such process found (process exists but is neither running nor stopped)\033[0m\n");

        free(file_path);
    }
    else
        fprintf(stderr, "\033[1;mERROR: bg : invalid syntax\033[0m\n");
}
