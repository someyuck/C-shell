#include "headers.h"

void warp(char **args, int num_args)
{
    char *temp;
    if (num_args == 1) // no args to warp command
    {
        temp = getcwd(NULL, 0);
        chdir(home_directory);
        strcpy(old_pwd, temp);

        char *cur = getcwd(NULL, 0);
        printf("%s\n", cur);
        free(cur);
        free(temp);
    }

    for (int i = 1; i < num_args; i++)
    {
        if (strcmp(args[i], "~") == 0)
        {
            temp = getcwd(NULL, 0);
            chdir(home_directory);
            strcpy(old_pwd, temp);
            free(temp);
        }
        else if (strcmp(args[i], "-") == 0)
        {
            if (old_pwd[0] == '\0')
            {
                printf("OLDPWD not set\n");
                continue;
            }
            else
            {
                temp = getcwd(NULL, 0);
                chdir(old_pwd);
                strcpy(old_pwd, temp);
                free(temp);
            }
        }
        else if (args[i][0] == '~' && strlen(args[i]) > 1) // paths rel to home dir of shell
        {
            args[i] += 1; // skip the starting '~'
            int total_path_length = strlen(home_directory) + strlen(args[i]);
            char *total_path = (char *)malloc(sizeof(char) * (total_path_length + 1));
            strcpy(total_path, home_directory);
            strcat(total_path, args[i]);

            temp = getcwd(NULL, 0);
            chdir(total_path);
            strcpy(old_pwd, temp);
            free(total_path);
            free(temp);
        }

        else
        {
            temp = getcwd(NULL, 0);
            chdir(args[i]);
            strcpy(old_pwd, temp);
            free(temp);
        }
        char *cur = getcwd(NULL, 0);
        printf("%s\n", cur);
        free(cur);
    }
}