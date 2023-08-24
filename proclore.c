#include "headers.h"

void proclore(char **args, int num_args)
{
    int pid;
    if (num_args == 0)
    {
        pid = shell_pid;
    }

    // convert pid to string;

    int length_pid = snprintf(NULL, 0, "%d", pid);
    char *pid_string = (char *)malloc(sizeof(char) * (length_pid + 1));
    snprintf(pid_string, length_pid + 1, "%d", pid);

    // /proc/<pid>/stat
    char *file_path = (char *)malloc(sizeof(char) * (6 + strlen(pid_string) + 6));
    strcpy(file_path, "/proc/");
    strcat(file_path, pid_string);
    strcat(file_path, "/stat");


    printf("file path: %s\n", file_path);

    FILE *fp = fopen(file_path, "r");
    if(fp == NULL) printf("yes\n");
    char* buffer = (char*)malloc(sizeof(char)* 4096);
    fread(buffer, sizeof(char), 1000, fp);
    fclose(fp);

    char status[3];
    long process_group;
    int tpgid;
    unsigned long virtual_memory;
    char *executable_path;

    char *token = NULL;
    int index = 0;

    while (index == 0 || token != NULL)
    {
        if (index == 0)
            token = strtok(buffer, " ");
        else
            token = strtok(NULL, " ");

        if (index == 2) // state
        {
            status[0] = token[0];
            status[1] = '?'; // dont know fg/bg yet
            status[2] = '\0';
        }
        else if (index == 4) // group id
        {
            char *end;
            process_group = strtol(token, NULL, 10);
        }
        else if (index == 7) // foreground process group id of terminal
        {
            char *end;
            tpgid = strtol(token, NULL, 10);
        }
        else if (index == 22) // virtual memory
        {
            char *end;
            virtual_memory = strtoul(token, NULL, 10);
        }
    }

    // find if it's an fg or bg process
    if (tpgid == process_group) // foreground process
        status[1] = '+';
    else // background process
        status[1] = '\0';

    strcpy(file_path, "/proc/");
    strcat(file_path, pid_string);
    strcat(file_path, "/exe");

    fp = fopen(file_path, "r");
    fread(buffer, sizeof(char), 4096, fp);
    fclose(fp);

    printf("pid : %d\n", pid);
    printf("Process Status : %s\n", status);
    printf("Process Group : %ld\n", process_group);
    printf("Virtual Memory : %lu\n", virtual_memory);

    printf("%s\n", buffer);
}
