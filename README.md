* Author: Claudia Zeng (4493821), Kerry Mo (4583522)
* Date: 01/22/2019
* CS 170 Project 1

### Description

We implemented a basic shell in C++ that would accept input from the user and execute commands that are given properly. 
It supports meta-characters ’< ’, ’>’, ’|’, and ’&’, correspondingly redirect standard input or output of commands to files, pipe the output of commands to other commands, and put commands in the background. The user can use `-n` option to hide the command prompt when execute the shell. Typing `Ctrl-D` would exit the shell program.

After the user gives a command, we first check if the the arguments are valid. If not, it will output an error and prompt to ask for the next input. If it is valid, we then fork a child process to execute the command. The parent process will handle background processes and wait for the child to finish.   


* Pipe

We first parse the command string into several subcommands. For each subcommand (except for the last one), we create a pipe and fork a child process that closes stdin (file descriptor 0) or stdout (file descriptor 1), opens the corresponding pipes, and use `dup2()` to redirect the previous  file descriptor. After calling `dup2()`, it closes the old file descriptor and execute the subcommand like normal command without pipe sign. The parent process would recursively call the pipe function with the next subcommand and .

* Input/Output Redirection

We first open (or create) the input or output file once when detect the corresponding characters.
