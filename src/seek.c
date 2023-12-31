#include "headers.h"

void seek(char **args, int num_args)
{
    int flag_status[3] = {0, 0, 0}; // 0 for f, 1 for d, 2 for e

    if (num_args == 1)
    {
        fprintf(stderr, "\033[1;31m'seek' requires arguments.\033[0m\n");
        return;
    }

    for (int i = 1; i < num_args; i++)
    {
        if (strcmp(args[i], "-f") == 0)
            flag_status[0] = 1;
        else if (strcmp(args[i], "-d") == 0)
            flag_status[1] = 1;
        else if (strcmp(args[i], "-e") == 0)
            flag_status[2] = 1;
        else if (strcmp(args[i], "-de") == 0 || strcmp(args[i], "-ed") == 0)
        {
            flag_status[1] = 1;
            flag_status[2] = 1;
        }
        else if (strcmp(args[i], "-fe") == 0 || strcmp(args[i], "-ef") == 0)
        {
            flag_status[0] = 1;
            flag_status[2] = 1;
        }
        else if (strcmp(args[i], "-fd") == 0 || strcmp(args[i], "-df") == 0)
        {
            flag_status[0] = 1;
            flag_status[1] = 1;
        }
        else if (strcmp(args[i], "-def") == 0 || strcmp(args[i], "-dfe") == 0 || strcmp(args[i], "-edf") == 0 || strcmp(args[i], "-efd") == 0 || strcmp(args[i], "-fde") == 0 || strcmp(args[i], "-fed") == 0)
        {
            flag_status[0] = 1;
            flag_status[1] = 1;
            flag_status[2] = 1;
        }
    }

    // error if both -d and -f flags are present
    if (flag_status[0] == 1 && flag_status[1] == 1)
    {
        fprintf(stderr, "\033[1;31mInvalid flags!\033[0m\n");
        return;
    }

    int is_target_mentioned = 0;
    char *target_dir;

    if (num_args == 2)
    {
        is_target_mentioned = 0;
    }
    else
    {
        // to check if target has been given, check for last two args, since the format for testing will always be "seek <flags> <search> <target_dir>"
        if (args[num_args - 2][0] == '-' && args[num_args - 1][0] != '-') // no need to explicitly check last condn but still
            is_target_mentioned = 0;
        else if (args[num_args - 2][0] != '-' && args[num_args - 1][0] != '-')
            is_target_mentioned = 1;
    }

    if (is_target_mentioned == 0)
        target_dir = getcwd(NULL, 0);
    else
    {
        if (strcmp(args[num_args - 1], "~") == 0) // home directory
        {
            target_dir = (char *)malloc(sizeof(char) * (strlen(home_directory) + 1));
            strcpy(target_dir, home_directory);
        }
        else if (strcmp(args[num_args - 1], "-") == 0) // prev pwd
        {
            if (old_pwd[0] == '\0')
            {
                printf("OLDPWD not set\n");
                return;
            }
            else
            {
                target_dir = (char *)malloc(sizeof(char *) * (strlen(old_pwd) + 1));
                strcpy(target_dir, old_pwd);
            }
        }
        else if (strlen(args[num_args - 1]) > 1 && args[num_args - 1][0] == '~') // path rel to home dir
        {
            args[num_args - 1] += 1;
            target_dir = (char *)malloc(sizeof(char) * (strlen(home_directory) + strlen(args[num_args - 1]) + 1));
            strcpy(target_dir, home_directory);
            strcat(target_dir, args[num_args - 1]);
            args[num_args - 1] -= 1; //  modify in peek also
        }
        else if (args[num_args - 1][0] != '/') // relative to cwd
        {
            // cheap trick to get absolute path of target dir (for if relative paths like .. are used)
            char *cwd_save = getcwd(NULL, 0);
            chdir(args[num_args - 1]);
            target_dir = getcwd(NULL, 0);
            chdir(cwd_save);
            free(cwd_save);
        }
        else // absolute path
        {
            target_dir = (char *)malloc(sizeof(char) * (strlen(args[num_args - 1]) + 1));
            strcpy(target_dir, args[num_args - 1]);
        }
    }

    int num_file_matches = 0;
    int num_dir_matches = 0;
    char *match_if_e_flag = NULL;

    // search recursively
    char *cwd_save = getcwd(NULL, 0);
    chdir(target_dir);
    printf("target: %s\n", getcwd(NULL, 0));
    if (is_target_mentioned == 0)
        seekRecursive(args[num_args - 1], target_dir, target_dir, &num_file_matches, &num_dir_matches, &match_if_e_flag, flag_status);
    else if (is_target_mentioned == 1)
        seekRecursive(args[num_args - 2], target_dir, target_dir, &num_file_matches, &num_dir_matches, &match_if_e_flag, flag_status);
    chdir(cwd_save);
    free(cwd_save);

    if ((flag_status[0] == 1 && num_file_matches == 0) || (flag_status[1] == 1 && num_dir_matches == 0) || (flag_status[0] == 0 && flag_status[1] == 0 && num_file_matches == 0 && num_dir_matches == 0))
    {
        printf("No matches found!\n");
    }

    struct stat *stat_ptr;
    if (flag_status[2] == 1)
    {
        stat_ptr = (struct stat *)malloc(sizeof(struct stat));
        int ret = stat(match_if_e_flag, stat_ptr);
        if (ret == -1)
        {
            fprintf(stderr, "\033[1;31mERROR : stat : errno(%d) : %s\033[0m\n", errno, strerror(errno));
            if (match_if_e_flag != NULL)
                free(match_if_e_flag);
            free(stat_ptr);
            free(target_dir);
            return;
        }
    }

    if (flag_status[0] == 1 && flag_status[1] == 0 && flag_status[2] == 1 && num_file_matches == 1)
    {
        if (stat_ptr->st_mode & S_IRUSR)
        {
            FILE *fp = fopen(match_if_e_flag, "r");
            char buf[100];
            while (fgets(buf, 100, fp) != NULL)
                printf("%s", buf);
            fclose(fp);
        }
        else
            printf("Missing permissions for task!\n");
    }
    else if (flag_status[0] == 0 && flag_status[1] == 1 && flag_status[2] == 1 && num_dir_matches == 1)
    {
        if (stat_ptr->st_mode & S_IXUSR)
        {
            char *temp = getcwd(NULL, 0);
            chdir(match_if_e_flag);
            strcpy(old_pwd, temp);
            free(temp);
        }
        else
            printf("Missing permissions for task!\n");
    }
    else if (flag_status[0] == 0 && flag_status[1] == 0 && flag_status[2] == 1)
    {
        if (num_file_matches == 1 && num_dir_matches == 0)
        {
            if (stat_ptr->st_mode & S_IRUSR)
            {
                FILE *fp = fopen(match_if_e_flag, "r");
                char buf[100];
                while (fgets(buf, 100, fp) != NULL)
                    printf("%s", buf);
                fclose(fp);
            }
            else
                printf("Missing permissions for task!\n");
        }
        else if (num_file_matches == 0 && num_dir_matches == 1)
        {
            if (stat_ptr->st_mode & S_IXUSR)
            {
                char *temp = getcwd(NULL, 0);
                chdir(match_if_e_flag);
                strcpy(old_pwd, temp);
                free(temp);
            }
            else
                printf("Missing permissions for task!\n");
        }
    }

    if (match_if_e_flag != NULL)
        free(match_if_e_flag);

    if (flag_status[2] == 1)
    {
        free(stat_ptr);
    }
    free(target_dir);
}

void seekRecursive(char *toSearch, char *OGTargetDirectory, char *targetDirectory, int *num_file_matches, int *num_dir_matches, char **match_if_e_flag, int flags[3])
{
    struct dirent **entryNames;
    int numEntries = scandir(targetDirectory, &entryNames, NULL, NULL);
    if (numEntries == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: scandir : errno(%d) : %s\033[0m\n", errno, strerror(errno));
        return;
    }
    else if (numEntries == 0)
        return;

    char *cwd = getcwd(NULL, 0);
    char * cwd1 = getcwd(NULL, 0);
	char * cwd2 = get_current_dir_name();
	// printf("cwd1: [%s] cwd2: [%s]\n", cwd1, cwd2);
	free(cwd1);
	free(cwd2);
    // printf("cwd : [%s]\n", cwd);
    for (int i = 0; i < numEntries; i++)
    {
        if (strcmp(entryNames[i]->d_name, ".") == 0 || strcmp(entryNames[i]->d_name, "..") == 0)
        {
            continue;
        }
            // printf("entry : [%s]\n", entryNames[i]->d_name);

        char *entryFullPath = (char *)malloc(sizeof(char) * (strlen(cwd) + 1 + strlen(entryNames[i]->d_name) + 1));
        strcpy(entryFullPath, cwd);
        strcat(entryFullPath, "/");
        strcat(entryFullPath, entryNames[i]->d_name);

        char *pathRelToTarget = (char *)malloc(sizeof(char) * (1 + strlen(entryFullPath) - strlen(OGTargetDirectory) + 1));
        strcpy(pathRelToTarget, "."); // . being rel to target dir
        strcat(pathRelToTarget, entryFullPath + strlen(OGTargetDirectory));

        // printf("[%s] [%s] [%s] [%s]\n", entryFullPath, pathRelToTarget, OGTargetDirectory, toSearch);
        struct stat *stat_ptr = (struct stat *)malloc(sizeof(struct stat));
        int ret = stat(entryFullPath, stat_ptr);
        if (ret == -1)
        {
            fprintf(stderr, "\033[1;31mERROR : stat : errno(%d) : %s [%s]\033[0m\n", errno, strerror(errno), entryFullPath);
            free(entryFullPath);
            free(pathRelToTarget);
            free(stat_ptr);
            continue;
        }
        int is_match = 0;
        int j = 0;
        while (j < strlen(toSearch) && j < strlen(entryNames[i]->d_name))
        {
            if (toSearch[j] != (entryNames[i]->d_name)[j])
                break;
            j++;
        }
        if ((toSearch[j] == '\0' && (entryNames[i]->d_name)[j] == '\0') || (j == strlen(toSearch) && j < strlen(entryNames[i]->d_name) && (entryNames[i]->d_name)[j] == '.'))
        {
            is_match = 1;
        }

        if (is_match == 1)
        {
            // a file is non-directory match
            if (S_ISDIR(stat_ptr->st_mode))
                (*num_dir_matches)++;
            else
                (*num_file_matches)++;

            if (flags[0] == 1 && flags[1] == 0 && flags[2] == 0)
            {
                if (!S_ISDIR(stat_ptr->st_mode))
                    printf("\033[1;32m%s\033[0m\n", pathRelToTarget);
            }
            else if (flags[0] == 1 && flags[1] == 0 && flags[2] == 1)
            {
                if (!S_ISDIR(stat_ptr->st_mode))
                {
                    printf("\033[1;32m%s\033[0m\n", pathRelToTarget);
                    if (*match_if_e_flag != NULL)
                    {
                        free(*match_if_e_flag);
                    }
                    *match_if_e_flag = (char *)malloc(sizeof(char) * (strlen(entryFullPath) + 1));
                    strcpy(*match_if_e_flag, entryFullPath); // overwrite latest match, no need to care for how many matches were found.
                }
            }
            else if (flags[0] == 0 && flags[1] == 1 && flags[2] == 0)
            {
                if (S_ISDIR(stat_ptr->st_mode))
                    printf("\033[1;34m%s\033[0m\n", pathRelToTarget);
            }
            else if (flags[0] == 0 && flags[1] == 1 && flags[2] == 1)
            {
                if (S_ISDIR(stat_ptr->st_mode))
                {
                    printf("\033[1;34m%s\033[0m\n", pathRelToTarget);
                    if (*match_if_e_flag != NULL)
                    {
                        free(*match_if_e_flag);
                    }
                    *match_if_e_flag = (char *)malloc(sizeof(char) * (strlen(entryFullPath) + 1));
                    strcpy(*match_if_e_flag, entryFullPath); // overwrite latest match, no need to care for how many matches were found.
                }
            }
            else if (flags[0] == 0 && flags[1] == 0 && flags[2] == 1)
            {
                if (S_ISDIR(stat_ptr->st_mode))
                    printf("\033[1;34m%s\033[0m\n", pathRelToTarget);
                else
                    printf("\033[1;32m%s\033[0m\n", pathRelToTarget);

                if (*match_if_e_flag != NULL)
                {
                    free(*match_if_e_flag);
                }
                *match_if_e_flag = (char *)malloc(sizeof(char) * (strlen(entryFullPath) + 1));
                strcpy(*match_if_e_flag, entryFullPath); // overwrite latest match, no need to care for how many matches were found.
            }
            else if (flags[0] == 0 && flags[1] == 0 && flags[2] == 0)
            {
                if (S_ISDIR(stat_ptr->st_mode))
                    printf("\033[1;34m%s\033[0m\n", pathRelToTarget);
                else
                    printf("\033[1;32m%s\033[0m\n", pathRelToTarget);
            }
        }

        if (S_ISDIR(stat_ptr->st_mode)) // recursively seach down the tree if entry is a directory
        {
            chdir(entryNames[i]->d_name);
            seekRecursive(toSearch, OGTargetDirectory, entryFullPath, num_file_matches, num_dir_matches, match_if_e_flag, flags);
            chdir("..");
        }

        free(entryFullPath);
        free(pathRelToTarget);
        free(stat_ptr);
    }
    free(cwd);
}
