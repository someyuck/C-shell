#include "headers.h"

int in_redirect(char *fname, int *file_fd, int *stdin_copy_fd)
{
    int fd = open(fname, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "\033[1;31mNo such input file found!\033[0m\n");
        return 0; // don't execute comm in main()
    }
    *file_fd = fd;
    int stdin_copy = dup(0);
    if (stdin_copy == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    *stdin_copy_fd = stdin_copy;
    int ret = dup2(fd, 0); // remember to close fd later and dup2 back the copy onto stdin.
    if (ret == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup2 : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    return 1;
}

int out_redirect(char *fname, int *file_fd, int *stdout_copy_fd)
{
    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: open : errno(%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    *file_fd = fd;
    int stdout_copy = dup(1);
    if (stdout_copy == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    *stdout_copy_fd = stdout_copy;
    int ret = dup2(fd, 1); // remember to close fd later and dup2 back the copy onto stdout.
    if (ret == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup2 : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    return 1;
}

int outc_redirect(char *fname, int *file_fd, int *stdout_copy_fd)
{
    int fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: open : errno(%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    *file_fd = fd;
    int stdout_copy = dup(1);
    if (stdout_copy == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    *stdout_copy_fd = stdout_copy;
    int ret = dup2(fd, 1); // remember to close fd later and dup2 back the copy onto stdout.
    if (ret == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup2 : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    return 1;
}

int dup2helper(int fd1, int fd2)
{
    int ret = dup2(fd1, fd2);
    if (ret == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup2 : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        return 0; // don't execute comm in main()
    }
    return 1;
}

void handle_redirection_and_execute(shell_command_data_ptr comm)
{
    int fin = -1;
    int fout = -1;
    int stdin_copy = 0;
    int stdout_copy = 1;
    int ret_in = -1;
    int ret_out = 1;
    if (comm->in_redir_fname != NULL)
        ret_in = in_redirect(comm->in_redir_fname, &fin, &stdin_copy);
    if (comm->out_redir_fname != NULL && comm->out_concat == 0)
        ret_out = out_redirect(comm->out_redir_fname, &fout, &stdout_copy);
    if (comm->out_redir_fname != NULL && comm->out_concat == 1)
        ret_out = outc_redirect(comm->out_redir_fname, &fout, &stdout_copy);

    if (ret_in != 0 && ret_out != 0) // if no errors while redirecting
        execute(comm);

    if (fin != -1)
    {
        close(fin);
        while (dup2helper(stdin_copy, 0) == 0)
        {
            printf("#");
        }
        close(stdin_copy);
    }
    if (fout != -1)
    {
        close(fout);
        while (dup2helper(stdout_copy, 1) == 0)
        {
            printf("$");
        }
        close(stdout_copy);
    }
}