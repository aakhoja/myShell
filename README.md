This program is created for the purpose of a project given in the class SE 4348.005 Operating System Concepts.

The program is called myshell and it does exactly what its called. Its a shell process. 

To Compile : gcc -o main myshell.c

The shell process can operate on multiple commands.

-   exit : to exit the shell process
-   pwd : to print current directory
-   clear : to clear the current screen
-   cd : to change current directory 

It also handles envinronmental varaibles with the commands 'setenv' and 'unsetenv'

The shell can also handle child processes.

Input and Output Redirection are 'cmd' 'args' 'input' 'output'

Also handles background process execution using &

Piping implements using 'args' | 'args' via pipe

SIGINT signal Ctrl+C is press - shell exits 


- There is a make file included in this project. In order to get the execution file. please enter 'make myshell' on command line and then execute the myshell by the command './myshell' . You will then be entered into a shell based called myshell. To terminate please enter 'exit' at any moment.


