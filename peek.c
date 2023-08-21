#include "headers.h"

void peek(char **args, int num_args)
{
    int flag_status[] = {0, 0}; // representing whether flags -a and -l are present or not; 0 meaning no, 1 meaning yes.

    // int option;
    // args += 1; // skip the 'peek' keyword so the only possible non-option argument is the path name

    // while ((option = getopt(num_args - 1, args, "la")) != -1) // && != '?' for error handling?
    // {
    //     printf("getopt: %c\n", option);
    //     if (option == 'a')
    //         flag_status[0] = 1;
    //     else if (option == 'l')
    //         flag_status[1] = 1;
    // }

    for (int i = 1; i < num_args; i++)
    {
        if (strcmp(args[i], "-l") == 0)
        {
            flag_status[1] = 1;
        }
        else if (strcmp(args[i], "-a") == 0)
        {
            flag_status[0] = 1;
        }
        else if (strcmp(args[i], "-la") == 0 || strcmp(args[i], "-al") == 0)
        {
            flag_status[0] = 1;
            flag_status[1] = 1;
        }
    }

    printf("flags: %d %d\n", flag_status[0], flag_status[1]);

    int is_path_arg_present;
    if ((flag_status[0] + flag_status[1] == 0 && num_args == 1) || (flag_status[0] + flag_status[1] == 1 && num_args == 2))
    {
        is_path_arg_present = 0;
    }
    else if ((flag_status[0] + flag_status[1] == 0 && num_args == 2) || (flag_status[0] + flag_status[1] == 1 && num_args == 3))
    {
        is_path_arg_present = 1;
    }
    else if (flag_status[0] + flag_status[1] == 2)
    {
        if (strcmp(args[0], "-la") == 0 || strcmp(args[0], "-al") == 0)
        {
            if (num_args == 2)
                is_path_arg_present = 0;
            else if (num_args == 3)
                is_path_arg_present = 1;
        }
        else
        {
            if (num_args == 3)
                is_path_arg_present = 0;
            else if (num_args == 4)
                is_path_arg_present = 1;
        }
    }
    // num_args -= 1;

    // now last arg in args is pathname, so handle '~' and '-', rest are relative paths so that's fine
    char *dir_path;
    if (is_path_arg_present == 1)
    {
        if (strcmp(args[num_args - 1], "~") == 0) // home directory
        {
            dir_path = (char *)malloc(sizeof(char) * (strlen(home_directory) + 1));
            strcpy(dir_path, home_directory);
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
                dir_path = (char *)malloc(sizeof(char *) * (strlen(old_pwd) + 1));
                strcpy(dir_path, old_pwd);
            }
        }
        else if (strlen(args[num_args - 1]) > 1 && args[num_args - 1][0] == '~') // path rel to home dir
        {
            args[num_args - 1] += 1;
            dir_path = (char *)malloc(sizeof(char) * (strlen(home_directory) + strlen(args[num_args - 1]) + 1));
            strcpy(dir_path, home_directory);
            strcat(dir_path, args[num_args - 1]);
        }
        else // either absolute path or relative to cwd
        {
            dir_path = (char *)malloc(sizeof(char) * (strlen(args[num_args - 1]) + 1));
            strcpy(dir_path, args[num_args - 1]);
        }
    }
    else
    {
        dir_path = getcwd(NULL, 0);
    }

    DIR *dir_ptr = opendir(dir_path);
    struct dirent *entry;
    if (dir_ptr == NULL)
    {
        printf("unable to open directory: %s\n", dir_path);
    }
    else
    {
        char **entry_strings_list;
        int entry_strings_list_index = 0;

        if ((entry = readdir(dir_ptr)) == NULL)
        {
            printf("error in readdir\n");
        }
        else
        {
            rewinddir(dir_ptr);
            while ((entry = readdir(dir_ptr)) != NULL)
            {
                if (flag_status[0] == 0 && entry->d_name[0] == '.') // skip hidden files, '.' and '..' if '-a' flag not there
                    continue;

                char *entry_full_path = (char *)malloc(sizeof(char *) * (strlen(dir_path) + 1 + strlen(entry->d_name) + 1));
                strcpy(entry_full_path, dir_path);
                strcat(entry_full_path, "/");
                strcat(entry_full_path, entry->d_name);

                struct stat *entry_stat_ptr = (struct stat *)malloc(sizeof(struct stat));
                stat(entry_full_path, entry_stat_ptr);
                
                char *entry_string;
                if(flag_status[1] == 1)
                {
                    char perms[11];
                    perms[10] = '\0';

                    if (S_ISDIR(entry_stat_ptr->st_mode)) perms[0] = 'd';
                    else if(S_ISLNK(entry_stat_ptr->st_mode)) perms[0] = 'l';
                    else perms[0] = '-';
                    
                    // user perms
                    if(entry_stat_ptr->st_mode & S_IRUSR) perms[1] = 'r';
                    else perms[1] = '-';

                    if(entry_stat_ptr->st_mode & S_IWUSR) perms[2] = 'w';
                    else perms[2] = '-';

                    if(entry_stat_ptr->st_mode & S_IXUSR) perms[3] = 'x';
                    else perms[3] = '-';

                    // group perms
                    if(entry_stat_ptr->st_mode & S_IRGRP) perms[4] = 'r';
                    else perms[4] = '-';

                    if(entry_stat_ptr->st_mode & S_IWGRP) perms[5] = 'w';
                    else perms[5] = '-';

                    if(entry_stat_ptr->st_mode & S_IXGRP) perms[6] = 'x';
                    else perms[6] = '-';

                    // others perms
                    if(entry_stat_ptr->st_mode & S_IROTH) perms[7] = 'r';
                    else perms[7] = '-';

                    if(entry_stat_ptr->st_mode & S_IWOTH) perms[8] = 'w';
                    else perms[8] = '-';

                    if(entry_stat_ptr->st_mode & S_IXOTH) perms[9] = 'x';
                    else perms[9] = '-';

                    // number of links to file
                    int length_nlinks = snprintf(NULL, 0,   "%d", entry_stat_ptr->st_nlink);
                    char *nlinks = (char*)malloc(sizeof(char)*(length_nlinks + 1));
                    snprintf(nlinks, length_nlinks + 1, "%d", entry_stat_ptr->st_nlink);

                    // user name of owner
                    struct passwd *pw_user = getpwuid(entry_stat_ptr->st_uid);
                    char *uname = pw_user->pw_name;

                    // group name of owner
                    struct group *gr_group = getgrgid(entry_stat_ptr->st_gid);
                    char *gname = gr_group->gr_name;

                    // file size on bytes
                    int length_size = snprintf(NULL, 0, "%d", entry_stat_ptr->st_blocks * 512);
                    char *size_bytes = (char*)malloc(sizeof(char)*(length_size + 1));
                    snprintf(size_bytes, length_size + 1, "%d", entry_stat_ptr->st_blocks);

                    ;

                


                }

                if (S_ISDIR(entry_stat_ptr->st_mode))
                {
                    printf("\033[1;34m%s\033[0m ", entry->d_name);
                }
                else if (S_ISREG(entry_stat_ptr->st_mode))
                {
                    if (entry_stat_ptr->st_mode & S_IXUSR)
                    {
                        printf("\033[1;32m%s\033[0m ", entry->d_name);
                    }
                    else
                    {
                        printf("\033[1;37m%s\033[0m ", entry->d_name);
                    }
                }

                free(entry_full_path);
                free(entry_stat_ptr);
            }
            printf("\n");
        }
    }
    closedir(dir_ptr);
    free(dir_path);
}