The file "smallsh.c" imitates a mini bash shell. To compile, run

gcc -o smallsh smallsh.c

The shell has a commandline starting with ": "
The shell has three built in functionalities

1. exit
exits the shell

2. cd
changes the working directory

3. status
returns the most recent exit status or terminating signal


The shell also handles built in bash commands such as "ls" and "echo"
by using fork(), exec(), and waitpid().

Adding a "&" at the end of the command will have the command run in the background

The shell handles signals, with SIGINT stopping the foreground process and 
SIGTSTP toggling foreground mode. Foreground mode ignores "&".

The shell also takes into account standard input and output ( > & < )

To run a grading script for this code, run ./p3gradingscript
If issues arise, try executing dos2unix to reformat
