# SPAGSHELL

Implementation of a basic LINUX shell in C.

Features
--------


* Basic commands: `exit`, `pwd`, `clear` , `help` and `cd`
* Program invocation with forking and child processes
* Piping implemented (`<cmd1> | <cmd2>`) via `pipe` and `dup2` syscalls. Multiple piping is allowed.
* SIGINT signal when Ctrl-C is pressed (shell is not exited)
* Command history
* Colored prompt

Execution Guidelines
----------------------------
readline library is required to run this program
To install the readline library, open the terminal window and write
`sudo apt-get install libreadline-dev`

Output
-------

![](Test.gif)

