#include "headers.h"

int comparator(const void *a, const void *b)
{
    return strcmp(*((char **)a), *((char **)b));
}

void peek(char **args, int num_args)
{
    int flag_status[] = {0, 0}; // representing whether flags -a and -l are present or not; 0 meaning no, 1 meaning yes.

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
        if (strcmp(args[1], "-la") == 0 || strcmp(args[1], "-al") == 0)
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
        fprintf(stderr, "\033[1;31munable to open directory: %s\033[0m\n", dir_path);
    }
    else
    {
        char **entry_strings_list = NULL;
        int entry_strings_list_index = 0;
        long int total_blocks = 0;

        if ((entry = readdir(dir_ptr)) == NULL)
        {
            fprintf(stderr, "\033[1;31mERROR: readdir: errno(%d) : %s\033[0m\n", errno, strerror(errno));
        }
        else
        {
            rewinddir(dir_ptr);
            while ((entry = readdir(dir_ptr)) != NULL)
            {
                if (flag_status[0] == 0 && entry->d_name[0] == '.') // skip hidden files, '.' and '..' if '-a' flag not there
                    continue;

                entry_strings_list = (char **)realloc(entry_strings_list, sizeof(char *) * (entry_strings_list_index + 1));
                entry_strings_list[entry_strings_list_index] = (char *)malloc(sizeof(char) * (strlen(entry->d_name) + 1));
                strcpy(entry_strings_list[entry_strings_list_index++], entry->d_name);

                char *entry_full_path = (char *)malloc(sizeof(char *) * (strlen(dir_path) + 1 + strlen(entry->d_name) + 1));
                strcpy(entry_full_path, dir_path);
                strcat(entry_full_path, "/");
                strcat(entry_full_path, entry->d_name);

                struct stat *entry_stat_ptr = (struct stat *)malloc(sizeof(struct stat));
                stat(entry_full_path, entry_stat_ptr);

                total_blocks += (entry_stat_ptr->st_blocks) / 2;

                free(entry_full_path);
                free(entry_stat_ptr);
            }

            if (flag_status[1] == 1)
                printf("total %ld\n", total_blocks);

            // sort based on names
            qsort(entry_strings_list, entry_strings_list_index, sizeof(char *), comparator);
            // now add info based on -l flag

            for (int i = 0; i < entry_strings_list_index; i++)
            {
                char *entry_full_path = (char *)malloc(sizeof(char *) * (strlen(dir_path) + 1 + strlen(entry_strings_list[i]) + 1));
                strcpy(entry_full_path, dir_path);
                strcat(entry_full_path, "/");
                strcat(entry_full_path, entry_strings_list[i]);

                struct stat *entry_stat_ptr = (struct stat *)malloc(sizeof(struct stat));
                stat(entry_full_path, entry_stat_ptr);

                char *file_info_string;
                if (flag_status[1] == 1)
                {
                    char perms[11];
                    perms[10] = '\0';

                    if (S_ISREG(entry_stat_ptr->st_mode))
                        perms[0] = '-';
                    else if (S_ISDIR(entry_stat_ptr->st_mode))
                        perms[0] = 'd';
                    else if (S_ISCHR(entry_stat_ptr->st_mode))
                        perms[0] = 'c';
                    else if (S_ISBLK(entry_stat_ptr->st_mode))
                        perms[0] = 'b';
                    else if (S_ISFIFO(entry_stat_ptr->st_mode))
                        perms[0] = 'p';
                    else if (S_ISLNK(entry_stat_ptr->st_mode))
                        perms[0] = 'l';
                    else if ((entry_stat_ptr->st_mode & __S_IFMT) == __S_IFSOCK) // IS_SOCK not there in stat.h
                        perms[0] = 's';
                    else
                        perms[0] = '?';

                    // user perms
                    if (entry_stat_ptr->st_mode & S_IRUSR)
                        perms[1] = 'r';
                    else
                        perms[1] = '-';

                    if (entry_stat_ptr->st_mode & S_IWUSR)
                        perms[2] = 'w';
                    else
                        perms[2] = '-';

                    if (entry_stat_ptr->st_mode & S_IXUSR)
                        perms[3] = 'x';
                    else
                        perms[3] = '-';

                    // group perms
                    if (entry_stat_ptr->st_mode & S_IRGRP)
                        perms[4] = 'r';
                    else
                        perms[4] = '-';

                    if (entry_stat_ptr->st_mode & S_IWGRP)
                        perms[5] = 'w';
                    else
                        perms[5] = '-';

                    if (entry_stat_ptr->st_mode & S_IXGRP)
                        perms[6] = 'x';
                    else
                        perms[6] = '-';

                    // others perms
                    if (entry_stat_ptr->st_mode & S_IROTH)
                        perms[7] = 'r';
                    else
                        perms[7] = '-';

                    if (entry_stat_ptr->st_mode & S_IWOTH)
                        perms[8] = 'w';
                    else
                        perms[8] = '-';

                    if (entry_stat_ptr->st_mode & S_IXOTH)
                        perms[9] = 'x';
                    else
                        perms[9] = '-';

                    // number of links to file
                    int length_nlinks = snprintf(NULL, 0, "%5ld", entry_stat_ptr->st_nlink);
                    char *nlinks = (char *)malloc(sizeof(char) * (length_nlinks + 1));
                    snprintf(nlinks, length_nlinks + 1, "%5ld", entry_stat_ptr->st_nlink);

                    // user name of owner
                    struct passwd *pw_user = getpwuid(entry_stat_ptr->st_uid);
                    int uname_len = snprintf(NULL, 0, "%12s", pw_user->pw_name);
                    char *uname = (char*)malloc(sizeof(char)*(uname_len + 1));
                    snprintf(uname, uname_len + 1, "%12s", pw_user->pw_name);

                    // group name of owner
                    struct group *gr_group = getgrgid(entry_stat_ptr->st_gid);
                    int gname_len = snprintf(NULL, 0, "%12s", gr_group->gr_name);
                    char *gname = (char*)malloc(sizeof(char)*(gname_len + 1));
                    snprintf(gname, gname_len +1, "%12s", gr_group->gr_name);

                    // file size on bytes
                    int length_size = snprintf(NULL, 0, "%8ld", entry_stat_ptr->st_size); // %8ld to indent
                    char *size_bytes = (char *)malloc(sizeof(char) * (length_size + 1));
                    snprintf(size_bytes, length_size + 1, "%8ld", entry_stat_ptr->st_size);

                    // last modification time
                    struct tm *time_st = localtime(&(entry_stat_ptr->st_mtime));
                    char mod_date_time[13];
                    strftime(mod_date_time, 13, "%b %d %H:%M", time_st);

                    file_info_string = (char *)malloc(sizeof(char) * (strlen(perms) + 1 + strlen(nlinks) + 1 + strlen(uname) + 1 + strlen(gname) + 1 + strlen(size_bytes) + 1 + strlen(mod_date_time) + 1 + 1)); // spaces in between
                    strcpy(file_info_string, perms);
                    strcat(file_info_string, " ");
                    strcat(file_info_string, nlinks);
                    strcat(file_info_string, " ");
                    strcat(file_info_string, uname);
                    strcat(file_info_string, " ");
                    strcat(file_info_string, gname);
                    strcat(file_info_string, " ");
                    strcat(file_info_string, size_bytes);
                    strcat(file_info_string, " ");
                    strcat(file_info_string, mod_date_time);
                    strcat(file_info_string, " ");

                    // printf("\n%s", file_info_string);

                    free(nlinks);
                    free(size_bytes);
                }
                else
                {
                    file_info_string = (char *)malloc(sizeof(char) * 1);
                    file_info_string[0] = '\0';
                }

                if (S_ISDIR(entry_stat_ptr->st_mode))
                {
                    printf("%s\033[1;34m%s\033[0m ", file_info_string, entry_strings_list[i]);
                }
                else if (S_ISREG(entry_stat_ptr->st_mode) && (entry_stat_ptr->st_mode & S_IXUSR))
                {
                    printf("%s\033[1;32m%s\033[0m ", file_info_string, entry_strings_list[i]);
                }
                else if (S_ISLNK(entry_stat_ptr->st_mode))
                {
                    printf("%s\036[1;32m%s\033[0m ", file_info_string, entry_strings_list[i]);
                }
                else
                {
                    printf("%s\033[1;37m%s\033[0m ", file_info_string, entry_strings_list[i]);
                }
                if (flag_status[1] == 1)
                    printf("\n");

                free(entry_full_path);
                free(entry_stat_ptr);
            }
            if (flag_status[1] == 0)
                printf("\n");
        }
    }
    closedir(dir_ptr);
    free(dir_path);
}