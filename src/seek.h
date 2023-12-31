#ifndef __SEEK_H
#define __SEEK_H

void seek(char **args, int num_args);
void seekRecursive(char *toSearch, char *OGTargetDirectory, char *targetDirectory, int *num_file_matches, int *num_dir_matches, char **match_if_e_flag, int flags[3]);

#endif
