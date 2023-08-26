#include "headers.h"

char **readlines(const char *path, int *num_lines)
{
    FILE *fp = fopen(path, "r"); // need to read, but created if not exists, and wont be overwritten
    if (fp == NULL)
    {
        fprintf(stderr, "\033[1;31mfopen errno(%d): %s\033[0m\n", errno, strerror(errno));
        return NULL;
    }
    char buf[4096 * 20 + 1];
    char **lines = NULL;
    *num_lines = 0;

    fread(buf, sizeof(char), 4096 * 20, fp);
    fclose(fp);

    if (strcmp(buf, "") == 0) // empty file
    {
        return lines;
    }

    printf("buf: (%s)\n", buf);
    char *line = NULL;
    while ((*num_lines) == 0 || line != NULL)
    {
        if (*num_lines == 0)
            line = strtok(buf, "\n\r");
        else
            line = strtok(NULL, "\n\r");

        if (line == NULL)
            break;

        lines = (char **)realloc(lines, sizeof(char *) * ((*num_lines) + 1));
        lines[(*num_lines)] = (char *)malloc(sizeof(char) * (strlen(line) + 1));
        strcpy(lines[(*num_lines)++], line);
    }

    return lines;
}

void writelines(const char *path, char **lines, int num_lines)
{
    FILE *fp = fopen(path, "w");
    for (int i = 0; i < num_lines; i++)
    {
        fprintf(fp, "%s\n", lines[i]);
    }
    fclose(fp);
}
