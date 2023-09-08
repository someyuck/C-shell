[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/76mHqLr5)
# Description
The following code files are part of my implementation of Part A:
- ```main.c```
This file contains code for repeatedly calling the functions responsible for the prompt, taking input, checking for background processes that may have finished/died, parsing the input string, and executing all commands entered,  and then storing them in history. It also handles some bookkeeping in the form of some global variables.

- ```prompt.c```
This file implements the prompt (Specification 1 : Display Requirement), wherein it obtains the user and system names, along with the current working directory (cwd). It parses the cwd against the home directory to give a relative path if the cwd is in the shell's home directory, or the absolute path otherwise. It also handles part of the system commands requirements(Specification 6 : System commands), printing the last foregound process if it took more than 2s to complete. The output is printed with appropriate colour coding.

- ```parse_input.c```
This file's main purpose is to implement the input requirements (Specification 2 : Input Requirements) and processing the input, packaging for execution of the input commands. It defines the construction and destruction functions delcared in ```parse_input.h``` for the structure encapsulating the information of one command. For parsing input, it tokenizes the input string with respect to the ';', '&' and then the whitespace characters, in order. It bookkeeps information regarding the commands, and finally produces a list of structs containg the command's individual words, and whether it is to be executed in the foreground or background. It also helps in bookkeeping for some global variables.

- ```warp.c```
This file implements Specification 3 (warp), by taking in a list of command arguments, processing them to determine the course of action as defined in the project document. If given directories to "warp" to, it iterates over them, comparing the path to the home directory's and then uses the chdir() syscall to change over to it, also setting a global variable to keep track of the previous cwd.

- ```peek.c```
This file implements Specification 4 (peek), by first deciding if the path argument is present or not. If so, it processes the path to get the path of the intended directory, either absolute or relative. Otherwise it takes it to be the cwd. It opens the target directory, looping over its contents using ```readdir()``` to first make a list of all entries, sort it using ```qsort()``` and then reiterate over them, getting their ```stat``` structures to implement the flags. If the ```-a``` flag is present, it shows the hidden files and directories. If the ```-l``` flag is present, it processes the ```stat``` structure of each entry to determine its type, mode bits, link count, owner and group names, size in bytes, date of last modification and then print this information along with the entry's name (colour coded as asked); if absent then just the names are printed.

- ```pastevents.c```
This file implements Specification 5 (pastevents commands), implementing one function for the pastevents command and one for storing the commands entered into history (```history.txt```). For the first function, it gets the full path of the ```history.txt``` file, by comparing the home directory's path with the cwd's. It uses two utility functions declared in ```utils.c```, to read and write lines from and to the file. For execute, it gets the reuqired command string. parses it and then executes it by using the ```execute()`` function from ```utils.h```. For the storing function, it reads the history file, checks if it has maxed out (15 commands), then adds the new command to the list and writes it all to the file. It also handles the case if the current command was pastevents iteself.

- ```utils.c```
This file defines utility functions used elsewhere: ```trim()``` (used in ```parse_input.c```), ```readlines()``` and ```writelines()``` (used in ```pastevents.c```) and ```execute()``` (used in ```main.c``` and ```pastevents.c```).

- ```system_commands.c```
This file implements Specification 6 (system commands), having two functions, one for handling an entered system command and the other for handling background process exits. The first one forks a child process, where it uses ```execvp()``` to call the entered command. In the parent process, it uses ```waitpid()``` to track the child, suspending and timing if the child is a foreground process, and printing the PID of the child if it's a background one.The other function loops over a global list of PIDs to check for the latest ended background child process has ended. This is called in ```main.c:main()``` a number of times to capture all such processes.
 
- ```proclore.c```
This file implements Specification 7 (proclore), deiplaying the info of the process with the entered PID (or the shell's process if no PID was entered). It parses ```/proc/<pid>/stat``` and ```/proc/<pid>/exe``` to get therequired information of the process, and throws and error if it fails (say due to permission issues).

# Assumptions
1. ```proclore``` will be run only when the shell is called with sudo.
2. In ```pastevents```, I have added a ';' to the end of the last command entered in the input string in ```history.txt```, even if it wasn't originally entered as such.
3. In ```pastevents```, I don't store ```pastevents execute <n>``` if n is greater than the current number of commands in ```history.txt```, even though this command will fail.
3. Also in ```pastevents```, to compare the latest entered command with the last stored command, I do not use direct string comparison; the comparison I use ignores the difference in whitespaces inside either command string. So, ```sleep     5; ls &``` and ```sleep 5;ls&``` are the same commands.
5. The output of background processes will be printed on the terminal, and it may overlap with the prompts. In such cases, press ENTER to get a fresh prompt with clear view (although it would still work with the overlapped prompt).
6. The output of system commands like ```ls``` will not be coloured (as in the linux terminal).
7. For ```seek```, I have assumed the following flags to be valid : ```-f```, ```-d```, ```-e```, ```-fe```, ```-ef```, ```-de```, ```-ed```. Also multiple occurences of the same flag is allowed, for instance, ```seek -f -fe -f -f -e```. All other possibilities (and also of course ```-f ``` and ```-d``` together) give the ```Invalid flags!``` error.
8. For ```seek``` again, I assume that the user will not miss entering the file to be searched.