#include "headers.h"

void activities(char **args, int num_args)
{
    if (num_args == 1)
    {
        int flag = 0;

        // first sort the bg processes lexicographically by pids
        char **bg_pid_strings = NULL;
        int num_procs = 0;

        for (int i = 0; i < MAX_BG_PROCESSES_TRACKED; i++)
        {
            if (bg_proc_pids[i] != -1)
            {
                flag = 1;

                bg_pid_strings = (char **)realloc(bg_pid_strings, sizeof(char *) * (num_procs + 1));
                int length_pid = snprintf(NULL, 0, "%d", bg_proc_pids[i]);
                bg_pid_strings[num_procs] = (char *)malloc(sizeof(char) * (length_pid + 1));
                snprintf(bg_pid_strings[num_procs], length_pid + 1, "%d", bg_proc_pids[i]);
                num_procs++;
            }
        }

        // actually lexicographical sorting so 10 < 2
        qsort(bg_pid_strings, num_procs, sizeof(char *), comparator); // comparator from peek.h

        for (int i = 0; i < num_procs; i++)
        {
            char *file_path = (char *)malloc(sizeof(char) * (strlen("/proc/") + strlen(bg_pid_strings[i]) + strlen("/stat") + 1));
            strcpy(file_path, "/proc/");
            strcat(file_path, bg_pid_strings[i]);
            strcat(file_path, "/stat");

            FILE *fp = fopen(file_path, "r");
            if (fp == NULL)
            {
                fprintf(stderr, "\033[1;31mERROR: activities: can't access info of process with pid %s\033[0m\n", bg_pid_strings[i]);
                for (int j = 0; j < num_procs; j++)
                    free(bg_pid_strings[j]);
                free(bg_pid_strings);
                free(file_path);
                continue;
            }

            int pid;
            char pname[4096];
            pname[4095] = '\0';
            char state[2];
            state[1] = '\0';

            fscanf(fp, "%d %s %s", &pid, pname, state);
            fclose(fp);

            // remove parentheses from around pname
            pname[strlen(pname) - 1] = '\0';

            if (state[0] == 'R' || state[0] == 'S')
                printf("%d: %s - Running\n", pid, pname + 1);
            if (state[0] == 'T')
                printf("%d: %s - Stopped\n", pid, pname + 1);
            // else
            //     printf("%d %d: %s %s- Stopped\n", bg_proc_pids[i], pid, bg_proc_names[i] ,pname);

            free(file_path);
        }
        if (flag == 0)
            printf("No spawned process is currently running\n");

        for (int j = 0; j < num_procs; j++)
            free(bg_pid_strings[j]);
        if (bg_pid_strings != NULL)
            free(bg_pid_strings);
    }
    else
        fprintf(stderr, "\033[1;31mERROR: activities: invalid syntax\033[0m\n");
}
