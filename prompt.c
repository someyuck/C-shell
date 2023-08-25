#include "headers.h"

void prompt()
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
    int i = 0;
    while (i < home_dir_path_length && i < pwd_path_length)
    {
        if (home_directory[i] != present_working_directory[i])
        {
            is_pwd_in_home_dir = 0;
            break;
        }
        i++;
    }
    if (home_directory[i] == '\0' && present_working_directory[i] == '\0')
    {
        is_pwd_in_home_dir = -1;
    }
    else if (home_directory[i] == '\0' && present_working_directory[i] != '\0')
    {
        is_pwd_in_home_dir = 1;
    }
    else if (home_directory[i] != '\0' && present_working_directory[i] == '\0')
    {
        is_pwd_in_home_dir = 0;
    }

    char *relative_path_of_pwd;

    if (is_pwd_in_home_dir == 1)
    {
        // relative path is ~/<rest-of-path>, i.e. concatenate "~" with substring of
        // pwd starting from _home_dir_len as index
        relative_path_of_pwd = (char *)malloc(sizeof(char) * (1 + pwd_path_length - home_dir_path_length + 1 + 10)); // '~' + <relative path> + '\0' + extra
        strcpy(relative_path_of_pwd, "~");
        char *pwd_copy = present_working_directory;
        present_working_directory += home_dir_path_length;
        strcat(relative_path_of_pwd, present_working_directory);

        free(pwd_copy);
    }
    else if (is_pwd_in_home_dir == -1)
    {
        relative_path_of_pwd = (char *)malloc(sizeof(char) * (2));
        relative_path_of_pwd[0] = '~';
        relative_path_of_pwd[1] = '\0';

        free(present_working_directory);
    }
    else if (is_pwd_in_home_dir == 0)
    {
        // use full path i.e. present_working_directory
        relative_path_of_pwd = present_working_directory;
    }

    if (long_fg_process == NULL) // from system_commands.c, fg processes longer than 2s
    {
        printf("<\033[1;34m%s\033[0m@\033[1;32m%s\033[0m:\033[1;31m%s\033[0m> ", username, hostname, relative_path_of_pwd);
    }
    else{
        printf("<\033[1;34m%s\033[0m@\033[1;32m%s\033[0m:\033[1;31m%s\033[0m %s%ds> ", username, hostname, relative_path_of_pwd, long_fg_process, long_fg_process_duration);
        free(long_fg_process);
        long_fg_process = NULL;
        long_fg_process_duration = -1;
        long_fg_process_strlen = 0;
    }
    free(relative_path_of_pwd);
}
