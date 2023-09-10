#ifndef __REDIRECT_H
#define __REDIRECT_H

int in_redirect(char *fname, int *file_fd, int *stdin_copy_fd);
int out_redirect(char *fname, int *file_fd, int *stdout_copy_fd);
int outc_redirect(char *fname, int *file_fd, int *stdout_copy_fd);
void handle_redirection_and_execute(shell_command_data_ptr comm);

int dup2helper(int fd1, int fd2);

#endif