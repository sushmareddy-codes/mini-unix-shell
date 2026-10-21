
# MINI UNIX SHELL COMMAND INTERPRETER

## 1. Aim
To implement a mini UNIX shell using the C programming language
that supports command execution, piping, and input/output redirection.

## 2. Abstract
The Mini UNIX Shell Command Interpreter is a command-line application
developed using C and UNIX system calls. It accepts commands from
the user and executes programs. It supports piping to transfer data
between commands and input/output redirection using files.
This project demonstrates process management and inter-process
communication in an operating system.

## 3. Objectives
- To understand the working of a UNIX shell.
- To execute commands using fork() and execvp().
- To implement piping using pipe() and dup2().
- To implement input and output redirection.
- To understand process synchronization using waitpid().

## 4. Software Requirements
- Programming Language: C
- Operating System: Ubuntu Linux
- Compiler: GCC
- Editor: Visual Studio Code / GitHub Codespaces

## 5. Features
- Basic command execution
- Process creation
- Piping between two commands
- Input redirection
- Output redirection
- Append redirection
- Shell termination using exit

## 6. System Calls Used
- fork(): Creates a child process.
- execvp(): Executes the requested program.
- pipe(): Creates a communication channel.
- dup2(): Redirects input or output.
- open(): Opens or creates files.
- waitpid(): Waits for child processes.
- chdir(): Changes the current directory.

## 7. Algorithm
1. Display the mini-shell prompt.
2. Read a command from the user.
3. Split the command into tokens.
4. Check for piping and redirection operators.
5. Create child processes when required.
6. Execute commands using execvp().
7. Redirect input/output or connect commands using a pipe.
8. Wait for child processes to finish.
9. Repeat until the user enters exit.

## 8. Compilation and Execution
Compile:
gcc -Wall -Wextra mini_shell.c -o mini_shell

Run:
./mini_shell

## 9. Sample Commands
pwd
ls
ls | wc -l
echo Hello World > output.txt
cat output.txt
sort < output.txt
echo Second Line >> output.txt
exit

## 10. Result
The Mini UNIX Shell Command Interpreter was implemented in C.
It supports basic command execution, piping, input redirection,
output redirection, and append redirection.

## 11. Limitations
- Supports only one pipe between two commands.
- Does not support quoted arguments or wildcard expansion.
- Does not implement background execution.

## 12. Future Enhancements
- Support multiple pipes.
- Add command history.
- Support background processes.
- Add environment-variable expansion.

## 13. Conclusion
This project provides practical knowledge of UNIX system calls,
process creation, command execution, piping, and file redirection.
