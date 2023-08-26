#ifndef __UTILS_H
#define __UTILS_H

char** readlines(const char * path, int *num_lines);
void writelines(const char * path, char **lines, int num_lines);

#endif