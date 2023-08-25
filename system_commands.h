#ifndef __SYSTEM_COMMANDS_H
#define __SYSTEM_COMMANDS_H

void handle_sigchld(int sig);
void system_command(shell_command_data_ptr command_data_ptr);

#endif