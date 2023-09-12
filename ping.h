#ifndef __PING_H
#define __PING_H

void ping(char **args, int num_args);

void handle_CTRL_C(int signal);
void handle_CTRL_D();
void handle_CTRL_Z(int signal);

void assign_signal_handlers();

#endif