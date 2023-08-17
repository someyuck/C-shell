#include "headers.h"

int main()
{
    // Keep accepting commands
    
    char *home_directory = getcwd(NULL, 0);

    while (1)
    {
        // Print appropriate prompt with username, systemname and directory before accepting input
        prompt(home_directory);
        char input[4096];
        fgets(input, 4096, stdin);
    }


    free(home_directory);
}
