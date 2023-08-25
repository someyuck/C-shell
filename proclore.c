#include "headers.h"

void proclore(char **args, int num_args)
{
    pid_t pid;
    char *pid_string;
    if (num_args == 1)
    {
        pid = shell_pid;

        // convert pid to string;
        int length_pid = snprintf(NULL, 0, "%d", pid);
        pid_string = (char *)malloc(sizeof(char) * (length_pid + 1));
        snprintf(pid_string, length_pid + 1, "%d", pid);
    }
    else
    {
        pid_string = (char *)malloc(sizeof(char) * (strlen(args[1]) + 1));
        strcpy(pid_string, args[1]);
    }

    char *file_path = (char *)malloc(sizeof(char) * (6 + strlen(pid_string) + 6));
    strcpy(file_path, "/proc/");
    strcat(file_path, pid_string);
    strcat(file_path, "/stat");

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        printf("can't access info of process with pid %s\n", pid_string);
        return;
    }
    char *buffer = (char *)malloc(sizeof(char) * 4096);
    buffer[4095] = '\0';
    fread(buffer, sizeof(char), 4095, fp);
    fclose(fp);

    char status[3];
    long process_group;
    int tpgid;
    unsigned long virtual_memory;

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
            // char *end;
            process_group = strtol(token, NULL, 10);
        }
        else if (index == 7) // foreground process group id of terminal
        {
            // char *end;
            tpgid = strtol(token, NULL, 10);
        }
        else if (index == 22) // virtual memory
        {
            // char *end;
            virtual_memory = strtoul(token, NULL, 10);
        }

        index++;
    }
    free(buffer);

    // find if it's an fg or bg process
    if (tpgid == process_group) // foreground process
        status[1] = '+';
    else // background process
        status[1] = '\0';

    printf("pid : %s\n", pid_string);
    printf("Process Status : %s\n", status);
    printf("Process Group : %ld\n", process_group);
    printf("Virtual Memory : %lu\n", virtual_memory);

    // handling executable path
    strcpy(file_path, "/proc/");
    strcat(file_path, pid_string);
    strcat(file_path, "/exe");

    buffer = (char *)malloc(sizeof(char) * 4096);
    buffer[4095] = '\0';
    ssize_t exec_path_length = readlink(file_path, buffer, 4095);
    if(exec_path_length == 4095 || exec_path_length == -1){
        printf("readlink errorno: %d\n", errno);
        printf("error message: %s\n", strerror(errno));
        return;
    }
    buffer[exec_path_length] = '\0';
    
    // print relative path if it's in home directory's subtree, else absolute path
    int is_path_in_home_dir;
    int i = 0;
    while(i< strlen(home_directory) && i < exec_path_length)
    {
        if(home_directory[i] != buffer[i]){
            is_path_in_home_dir = 0;
            break;
        }
        i++;
    }
    if(home_directory[i] == '\0' && (buffer[i] != '\0' || buffer[i] == '\0')) // either home dir itself or inside home dir
    {
        is_path_in_home_dir = 1;
    }
    else if(home_directory[i] != '\0' && buffer[i] == '\0') // path is parent of home dir
    {
        is_path_in_home_dir = 0;
    }


    if(is_path_in_home_dir == 0) // print absolute path
    {
        printf("Executable path : %s\n", buffer);
    }
    else // print relative path
    {
        char *rel_exec_path = (char*)malloc(sizeof(char)*(1+ exec_path_length - strlen(home_directory) + 1));
        strcpy(rel_exec_path,"~");
        strcat(rel_exec_path, (buffer + strlen(home_directory)));
        printf("Executable path : %s\n", rel_exec_path);
        free(rel_exec_path);
    }

    free(buffer);

}
