# C shell

A shell written entirely in C, intending to mimic bash. I have implemented some commands and features, while other shell commands are directly executed using `exec`.

## Requirements
- `gcc`
- `make`

## Installation
Clone the repository and `cd` into it. Open a terminal in the directory and run `make`. This will create the executable binary, `a.out`. Run it as `./a.out` to launch the shell. The executable can be run in any directory in any number of terminals, and can even be called in itself.

## Features
This shell provides the following features:

### Prompt
The shell displays the current user's username and the system name in the prompt, separated by an `@`, along with the current working directory denoted by its path relative to the directory in which the shell was called, which itself becomes the shell's home directory (denoted by `~`). If the current working directory is outside the home directory, then its absolute path is shown.

If a foreground process took more than 2 seconds to execute, the prompt displays the process' name along with the time taken in seconds.

### Input
The shell supports any number of commands separated by semicolons (;) and ampersands (&), denoting foreground and background processes. It also supports I/O redirection and piping.
The shell accounts for random whitespaces and displays an error message if an erroneous command is entered.

### I/O Redirection
This shell also supports input-output redirection, used with the `<` (input), `>` (output/overwrite) and `>>` (append) operators, to redirect input from and redirect output to a user-specified file. The output operators create a new file if the given one doesn't exist, and the input operator raises an error.

### Piping
This shell also supports piping, denoted by the `|` operator, to pass the output of one command as input to another. It supports any number of pipes, with commands run sequentially from left to right.
The shell also supports redirection along with piping, using the bash convention in giving I/O redirection higher priority over piping.

### `warp`
This command changes the current working directory of the shell, just like `cd` in bash. The target directory's name is provided as argument to the command, otherwise it just moves to the home directory.
`warp` supports both relative and absolute paths, and by default, a non-absolute path will be treatedrelative to the current working directory.
`warp` also supports the `.` (current directory), `..` (parent directory), `~` (shell's home directory) and `-` (previous directory, if `warp` had been called before) flags.

If an erroneous path is given, `warp` does nothing and stays in the current directory.

`warp` supports multiple arguments, and sequentially changes into each directory mentioned.

After changing into a directory, `warp` prints its full path to the terminal.

Its usage is `warp <path/to/target/directory>`

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


### `proclore`
This command gives information on a process, whose process ID is passed as argument, printing its process status, process group, virtual memory, and the executable's path. The process status can take values `R`/`R+` (running), `S`/`S+` (sleeping) and `Z` (zombie), with the `+` denoting a foreground process.
Run the shell with `sudo` for it to work for all processes.

### `seek`
This command recursively searches for a file/directory (whose name must be given) in a specified target directory (or current directory if none given), returning a list of colour-coded paths relative to the target directory of all matches.

It supports the flags:
`-d`: looks for only directories
`-f`: looks for only files
`-e`: if a single file is found (and no directories are found if `-f` not used), it prints the content of the matched file; if a single directory is found (and no files are found if `-d` is not used) then it changes the current working directory to it.

Note that `-f` and `-d` cannot be used together.

Its usage is `seek <flags> <target> <path/to/target/directory>`


### `activities`
This command lists all (background) processes spawned by the shell, in lexicographical order, priniting the command name, process ID, and state (running or stopped).

### `ping`
This command is used to send signals to processes, with signals passed as a numeric argument (modulo-ed by 32).

Its usage is `ping <pid> <signal>`

### Other signals

Users can send signals via the keyboard as well:
- `Ctrl + C`: Interrupts any currently running foreground process using the SIGINT signal.
- `Ctrl + D`: Logs out of the shell after killing all processes.
- `Ctrl + Z`: Push the currently running foreground process to the background and change its state to 'Stopped'.

### `fg`
This command brings a background process (running or stopped) to the foreground.
Its usage is `fg <pid>`

### `bg`
This command changes the state of a stopped background process to running in the background.
Its usage is `bg <pid>`

### `neonate`
This command prints the process ID of the most recently created process on the system at a time interval in seconds, whose value is provided as an argument, printing constantly until the `X` key is pressed.
Its usage is `neonate -n <time-arg>`

### `iMan`
This command fetches man pages from the internet (http://man.he.net) and prints them to the terminal. This uses networking APIs to connect to the webserver (after doing DNS resolution) via a TCP socket, sending a GET request for the search query, and closing the socket. It processes the output text to print just the content from the fetched HTML page.
Its usage is `iMan <command-name>`

### System Commands
All other commands supported by other shells like bash (and installed in the user's system) can be executed. Any command entered other than the above implemented commands is passed to `execvp()` and thus executed. Note that if the output of the command is coloured then the output shown in the shell need not be coloured.
The shell supports both foreground and background processes, with status messages printed for background processes accordingly as the proccess exited successfully or failed.

# Codebase
The entirety of the codebase is in `src/`. Each functionality has its own `.h` and `.c` files, with the below files performing some core functionality:
- `headers.h`: includes all the header files used in the codebase and defines all the global variables used.
- `main.c`: the main program that runs the shell, setting things up and linking together components of the codebase.
- `prompt.c`: code for the prompt that is displayed once control reached the shell.
- `input_defs.h`: defines structs used to store commands and their arguments in a logical order, accounting for piping, redirection and foreground/background processes.
- `parse_input.c`: parses the input string passed at the prompt, breaking it up into the structs defined in `input_defs.h`.
- `redirect.c`: handles redirection, using the `dup()` family of functions.
- `pipeline.c`: handles piping, using the `dup()` family of functions.
- `utils.c`: defines some functions used throughout.
- `system_commands.c`: defines functions for executing system commands, handling errors as well.

# Assumptions
1. For ```ping```, entering negative pids will throw an error, as first of all they are invalid and calling `kill()` on negatives is undesirable, as e.g. on pid = -1, `kill()` with `SIGKILL` essentially logs us out of the system. Entering 0 sends a signal to the processes in the shell's process group, so that is allowed
2. For `iMan`, I have not processed HTML tags inside the actual content of the webpage, and have left them as it is. So, tags like `<STRONG>` may be here and there in the text.
3. The output of background processes will be printed on the terminal, and it may overlap with the prompts. In such cases, press ENTER to get a fresh prompt with clear view (although it would still work with the overlapped prompt).
4. For I/O redirection, I have assumed that the user will enter files that are in the current working directory, and have not implemented relative paths.
5. For ```ping```, entering negative pids will throw an error, as first of all they are invalid and calling `kill()` on negatives is undesirable, as e.g. on pid = -1, `kill()` with `SIGKILL` essentially logs us out of the system. Entering 0 sends a signal to the processes in the shell's process group, so that is allowed.


# Future Scope
1. Adding support for the keystrokes : `up-arrow`/`Ctrl+P`, `down-arrow`, `Tab` etc. as with usual terminal emulators.
2. More extensive error messages and text processing.
3. Bracket and quote expansion as in bash (Currently `sed` doesn't work with piping because I have not implemented quote expansion).
4. Adding support to relative paths for I/O redirection.