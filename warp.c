#include "headers.h"

void warp(char **args, int num_args)
{
    char *temp = getcwd(NULL, 0);
    if (num_args == 1) // no args to warp command
    {
        chdir(home_directory);
        printf("%s\n", home_directory);
        free(old_pwd);
        old_pwd = temp;
    }

    for (int i = 1; i < num_args; i++)
    {
        if (strcmp(args[i], "~") == 0)
        {
            chdir(home_directory);
            free(old_pwd);
            old_pwd = temp;
        }
        else if (strcmp(args[i], "-") == 0)
        {
            if (old_pwd == NULL)
            {
                printf("OLDPWD not set\n");
                continue;
            }
            else
            {
                chdir(old_pwd);
                free(old_pwd);
                old_pwd = temp;
            }
        }
        else if (args[i][0] == '~' && strlen(args[i]) > 1) // paths rel to home dir of shell
        {
            args[i] += 1; // skip the starting '~'
            int total_path_length = strlen(home_directory) + strlen(args[i]);
            char *total_path = (char *)malloc(sizeof(char) * total_path_length);
            strcpy(total_path, home_directory);
            strcat(total_path, args[i]);

            chdir(total_path);
            free(old_pwd);
            old_pwd = temp;
            free(total_path);
        }

        else
        {
            chdir(args[i]);
            free(old_pwd);
            old_pwd = temp;
        }
        char *cur = getcwd(NULL, 0);
        printf("%s\n", cur);
        free(cur);
    }
}