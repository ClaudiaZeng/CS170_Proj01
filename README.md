Author: Claudia Zeng (4493821), Kerry Mo (4583522)

Date: 01/22/2019

CS 170 Project 1

### Description

We implemented a basic shell in C++ that would accept input from the user and execute commands that are given properly. It supports meta-characters `< `, `>`, `|`, and `&`, correspondingly redirect standard input or output of commands to files, pipe the output of commands to other commands, and put commands in the background. The user can use `-n` option to hide the command prompt when execute the shell. Typing `Ctrl-D` would exit the shell program.

After the user gives a command, we first check if the the arguments are valid. If not, it will output an error and prompt to ask for the next input. If it is valid, we then fork a child process to execute the command. The parent process will handle background processes and wait for the child to finish. 

For executing the command, we first check whether or not it contains pipes. If the command contains one or more pipes, we first parse the command string into several subcommands then call the pipe function. If not, we need to check if the command contains input redirection only, output redirection only, both, or neither.

#### Input/Output Redirection

* Input redirection only

We first open the input file once we detect the `<` characters. Then we use `dup2()` to redirect stdin to the source file and close the newly opened file and use `execvp()` to execute the command.

* Output redirection only

We first open the output file (or create the output file if not exists) once we detect the `>` character. Then we use `dup2()` to redirect stdout to the destination file and close the newly opened file and use `execvp()` to execute the command.

* Both

If the command contains both input redirection and output redirectin, we then first parse the command to get the part with input redirection only and the output file name. Then do same as stated in the input redirection only then output redirection only.

* Neither

If neither input redirection/output redirection nor pipes exist, that means we only have one command, then we use `execvp()` to execute the command directly.


#### Pipe

For each subcommand (except for the last one), we create a pipe and send a `fork()` call creating a child process that closes stdin (file descriptor 0), uses `dup2()` to redirect stdin to the previous file descriptor 0 and redirect stdout to file descriptor 1. After calling `dup2()`, it closes the old file descriptor and execute the subcommand like normal command without pipe sign. The parent process would close unused ends of the pipe and recursively call the pipe function with the next subcommand and the current file descriptor 0.


