# C shell

A shell written entirely in C, intending to mimic bash. I have implemented some commands, and other shell commands are directly executed using `exec`.

## Installation
Clone the repository and `cd` into it. Open a terminal in the directory and run `make`. This will create the executable binary, `a.out`. Run it as `./a.out` to launch the shell. The executable can be run in any directory in any number of terminals, and can even be called in itself.

## Features
This shell provides the following features:

### Prompt
The shell displays the current user's username and the system name in the prompt, separated by an @, along with the current working directory denoted by its path relative to the directory in which the shell was called, which itself becomes the shell's home directory (denoted by `~`). If the current working directory is outside the home directory, then its absolute path is shown.

If a foreground process took more than 2 seconds to execute, the prompt displays the process' name along with the time taken in seconds.

### Input
The shell supports any number of commands separated by semicolons (;) and ampersands (&), denoting foreground and background processes. It also supports i/o redirection and piping (more on these later).
The shell accounts for random whitespaces and displays an error message if an erroneous command is entered.

### `warp`
This command changes the current working directory of the shell, just like `cd` in bash. The target directory's name is provided as argument to the command, otherwise it just moves to the home directory.
`warp` supports both relative and absolute paths, and by default, a non-absolute path will be treatedrelative to the current working directory.
`warp` also supports the `.` (current directory), `..` (parent directory), `~` (shell's home directory) and `-` (previous directory, if `warp` had been called before) flags.

If an erroneous path is given, `warp` does nothing and stays in the current directory.

`warp` supports multiple arguments, and sequentially changes into each directory mentioned.

After changing into a directory, `warp` prints its full path to the terminal.

### `peek`

This command, just like the `ls` command in bash, lists all the files and directories in the specified directory in lexicographical order. If no argument is given, it lists the contents of the current working directory.
It supports the following flags:
`-a`: displays all files and directories, including hidden ones
`-l`: displays additional information for each entry

Its usage is `peek <flags> <path/to/directory>`

Similar to warp, it supports `.`, `..`, `~`, `-`, relative and absolute paths and colour-codes the output. It does not however, support multiple arguments, and will peek at the current working directory instead.

### `pastevents`
This command is similar to the `history` command in bash, with additional festures. It persistently stores the 15 most recent command statements given as input to the shell.
If two consecutive commands are the same, the later one is not stored. `pastevents` itself and its derivatives (mentioned below) are not stored. Erroneous commands are stored, including `pastevents` derivatives.

- `pastevents purge`
This clears all the currently stored commands.
- `pastevents execute <n>`
This executes the _n<sup>th</sup>_ most recent command stored in history.

### System Commands
All other commands supported by other shells like bash (and installed in the user's system) can be executed. Any command entered other than the above implemented commands is passed to `execvp()` and thus executed. Note that if the output of the command is coloured then the output shown in the shell need not be coloured.
The shell supports both foreground and background processes, with status messages printed for background processes accordingly as the proccess exited successfully or failed.

### `proclore`
This command gives information on a process, whose process ID is passed as argument, printing its process status, process group, virtual memory, and the executable's path. The process status can take values `R`/`R+` (running), `S`/`S+` (sleeping) and `Z` (zombie), with the `+` denoting a foreground process.

### `seek`
This command recursively searches for a file/directory in a specified target directory (or current directory if none given), returning a list of colour-coded paths relative to the target directory of all matches.

It supports the flags:
`-d`: looks for only directories
`-f`: looks for only files
`-e`: if a single file is found (and no directories are found if `-f` not used), it prints the content of the matched file; if a single directory is found (and no files are found if `-d` is not used) then it changes the current working directory to it.

Note that `-f` and `-d` cannot be used together

Its usage is `seek <flags> <target> <path/to/target/directory>`

### I/O Redirection

### Piping

### `activities`

### `ping`

### Other signals

### `fg`

### `bg`

### `neonate`

### `iMan`

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
2. In ```pastevents```, I have added a ';' to the end of the last command (only if it wasn't entered as a background process) entered in the input string in ```history.txt```, even if it wasn't originally entered as such.
3. In ```pastevents```, I don't store ```pastevents execute <n>``` if n is greater than the current number of commands in ```history.txt```, even though this command will fail. However, if the command is `patevents` and the syntax is invalid, I store the command in history.
3. Also in ```pastevents```, to compare the latest entered command with the last stored command, I do not use direct string comparison; the comparison I use ignores the difference in whitespaces inside either command string. So, ```sleep     5; ls &``` and ```sleep 5;ls&``` are the same commands.
5. The output of background processes will be printed on the terminal, and it may overlap with the prompts. In such cases, press ENTER to get a fresh prompt with clear view (although it would still work with the overlapped prompt).
6. The output of system commands like ```ls``` will not be coloured (as in the linux terminal).
7. For ```seek```, I have assumed the following flags to be valid : ```-f```, ```-d```, ```-e```, ```-fe```, ```-ef```, ```-de```, ```-ed```. Also multiple occurences of the same flag is allowed, for instance, ```seek -f -fe -f -f -e```. All other possibilities (and also of course ```-f ``` and ```-d``` together) give the ```Invalid flags!``` error.
8. For ```seek``` again, I assume that the user will not miss entering the file to be searched.
9. For i/o redirection, I have assumed that the user will enter files that are in the current working directory, and have not implemented relative paths.
10. For i/o redirection : parse_input.c:204
11. For cases involving both pipelines and i/o redirection, I have given higher priority to redirection, i.e. any redirections made via pipes will not be considered for i/o flow and redirection operators (if present) will be used.
12. ```sed``` won't work with piping as I have not implemented the expansion of single quotes as in bash.
13. For `activities`, I am only displaying those (only background system) processes spawned by the shell that are _currently_ in the "running" (R) or "sleeping" (S)  or "stopped" (T) states. All other states and exited processed are treated as "stopped" and will not be displayed.
14. Also for ```activities```, since the doubt document mentions lexicographical ordering is by ```pid```, note that a process with pid ```10``` will appear before one with pid ```2```.
15. For ```ping```, entering negative pids will throw an error, as first of all they are invalid and calling `kill()` on negatives is undesirable, as e.g. on pid = -1, `kill()` with `SIGKILL` essentially logs us out of the system. Entering 0 sends a signal to the processes in the shell's process group, so that is allowed
16. For ```fg```, say if I push a foreground process to background with `Ctrl+Z`, then bring it back up again using `fg`, the time measured for use in the prompt (if the foreground process uses > 2 seconds), the time checked while it was first running in the foreground is separate from the time checked while it runs in the foreground the second time. Meaning that the first prompt after the `Ctrl+Z` will display the time it stayed in the foreground (if more than 2s) till the signal was sent, and then when `fg` is used to bring it back, the prompt after it exits (on completion or if we `Ctrl+Z` again) will show the time it consumed in the foreground the second time around, and NOT include the time for the previous run.
17. For `neonate`, I am spawning a child process which actually runs `neonate`, while its parent(the shell process) runs in raw mode to look for an input x. However, I have not implemented handling of the `Ctrl+C` and `Ctrl+Z` signals for this child process, and am also not tracking this new process for use in say `activities`, or the time feature in the prompt. Also note that since two processes are now running simultaneously, on giving a `Ctrl+C` or `Ctrl+Z` interrupt the "no process running right now" (since the child isn't being tracked) will be displayed twice, as it is received by both processes.
18. For `iMan`, I have not processed HTML tags inside the actual content of the webpage, and have left them as it is. So, tags like `<STRONG>` may be here and there in the text.