#include "headers.h"

void prompt(char *home_directory)
{
    // Do not hardcode the prmopt
    // printf("<Everything is a file> ");

    // handling user name and hostname

    char *username = getlogin();
    if (username == NULL)
        return;
    char *hostname = (char *)malloc(sizeof(char) * 4096);
    assert(hostname != NULL);
    gethostname(hostname, 4096);

    // handling pwd path

    char *present_working_directory = getcwd(NULL, 0);
    int home_dir_path_length = strlen(home_directory);
    int pwd_path_length = strlen(present_working_directory);
    int is_pwd_in_home_dir = 1;

    // 3 cases: if pwd is in tree of home dir (1), if pwd is home dir (-1), or if pwd is outside tree of home dir (0)

    // check if pwd is in tree of home dir
    int i;
    for (i = 0; i < home_dir_path_length; i++)
    {
        if (present_working_directory[i] != home_directory[i])
        {
            is_pwd_in_home_dir = 0;
            break;
        }
    }
    if (present_working_directory[i] == '\0') // both strings end meaning pwd is home dir
    {
        is_pwd_in_home_dir = -1;
    }

    char *relative_path_of_pwd;

    if (is_pwd_in_home_dir == 1)
    {
        // relative path is ~/<rest-of-path>, i.e. concatenate "~" with substring of
        // pwd starting from _home_dir_len as index
        relative_path_of_pwd = (char *)malloc(sizeof(char) * (1 + pwd_path_length - home_dir_path_length + 1)); // '~' + <relative path> + '\0'
        relative_path_of_pwd[0] = '~';

        int i;
        for (i = home_dir_path_length; i < pwd_path_length; i++)
        {
            relative_path_of_pwd[i - home_dir_path_length + 1] = present_working_directory[i];
        }

        relative_path_of_pwd[i] = '\0';
    }
    else if (is_pwd_in_home_dir == -1)
    {
        relative_path_of_pwd = (char *)malloc(sizeof(char) * (2));
        relative_path_of_pwd[0] = '~';
        relative_path_of_pwd[1] = '\0';
    }
    else if (is_pwd_in_home_dir == 0)
    {
        // use full path i.e. present_working_directory
        relative_path_of_pwd = present_working_directory;
    }

    printf("<\033[1;34m%s\033[0m@\033[1;32m%s\033[0m:\033[1;37m%s\033[0m> ", username, hostname, relative_path_of_pwd);

    free(present_working_directory);
}
