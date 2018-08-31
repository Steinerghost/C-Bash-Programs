//Author: Casey Ford
//Date: 5/27/18
//Class: CS344

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/****GLOBALS*******/
int SIGTSTP_flag = 0;   //A flag for the foreground-only mode. Defaults at off.

//Removes an element from a string array at a given index. The contents are shifted
//one to the left starting at the index location. The end location is set to null.
void remove_element(char** char_array, int index){
	int i;
	//For every possible array location starting at the given index
	for(i = index-1; i < 512; i++){
		//If the loop reaches to the end of the populated array
		if(char_array[i] == NULL){
			//Break out of the loop
			break;
		}
		//Shift the elements to the left
		char_array[i] = char_array[i+1];
	}
}

//Inserts a given pid int onto the end of a given int array
void insert_pid(int* pid_array, int my_pid){
	int i = 0;
	int array_length = 0;
	//Determine the size of the array by interating until the
	//loop hits zero.
	while(pid_array[i] != 0){
		array_length++;
		i++;
	}
	//Add the new pid onto the end of the array
	pid_array[array_length] = my_pid;
}

//Removes a pid from a given pid array at a given index
void remove_pid(int* pid_array, int index){
	int array_length = 0;
	int i = 0;
	//Determine the size of the array bu interating until the
	//loop hits zero.
	while(pid_array[i] != 0){
		array_length++;
		i++;
	}

	//Iterate through the populated array starting at the given index
	for(i = index; i < array_length-1; i++){
		//Shift the elements to the left
		pid_array[i] = pid_array[i+1];
	}

	//Set the last element to zero
	pid_array[i] = 0;
}

//A signal catcher for the SIGTSTP signal. The function toggles the
//foreground-only mode.
void catchSIGTSTP(int signo){
	//Create a message array for use in write
	char message[500];
	memset(message, '\0', sizeof(message));

	//If the mode is off
	if(SIGTSTP_flag == 0){
		//Print the start of the mode
		sprintf(message, "\nEntering foreground-only mode (& is now ignored)\n");
		write(STDOUT_FILENO, message, sizeof(message));
		//Set the flag to on
		SIGTSTP_flag = 1;
		//Print a new prompt line
		write(STDOUT_FILENO, ": ", 2);
	}
	//If the mode is on
	else if(SIGTSTP_flag == 1){
		//Print the end of the mdoe
		sprintf(message, "\nExiting foreground-only mode\n");
		write(STDOUT_FILENO, message, sizeof(message));
		//Set the flag to off
		SIGTSTP_flag = 0;
		//Print a new prompt line
		write(STDOUT_FILENO, ": ", 2);
	}
}

int main(){
	//Initalize signal handlers
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};

	ignore_action.sa_handler = SIG_IGN;
	SIGINT_action.sa_handler = SIG_DFL;

	//Setup the SIGTSTP handler to point to catchSIGTSTP, block all signals in sa_mask,
	//and restart input with SA_RESTART
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;

	//Disable SIGINT for the parent and all child processes
	//(Foreground will be reset to SIG_DFL later)
	sigaction(SIGINT, &ignore_action, NULL);
	//Call the SIGTSTP signal handler for the process
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	//Initalize variables
	char* buffer = NULL;                //User input
	size_t bufsize = 0;          //Size in bytes of user input
	int num_characters = -5;     //Size in characters of user input
	char* arg_array[512];        //Array for each arguement in a given user input
	char input[2048];            //The character limit for a given input

	int test_method = -5;        //Child exit int for use in checking background processes
	int pid_array[512];          //Contains the pid of any background processes

	char* token;                 //Token container for strtok()
	int i;                       //Iterater in for loops

	int exit_status = 0;         //Contains int values for exit status for use in the status command
	char signal_status[300];     //Contains the string for signal exit status for use in the status command

	//Loop until user wants to exit
	while(1){
		//Restart iterator
		i = 0;

		//While there are background processes that need to be reaped
		while(pid_array[i] != 0){
			//Initalize the return status for debugging
			test_method = -5;
			//Check each background process for completion
			int child_to_check = waitpid(pid_array[i], &test_method, WNOHANG);

			//If the process is done
			if(WIFEXITED(test_method) && child_to_check == pid_array[i]){
				//Print the exit value and remove the pid from the array
				printf("Background pid %d is done: exit value %d\n", pid_array[i], WEXITSTATUS(test_method));
				fflush(stdout);
				remove_pid(pid_array, i);
			}
			//If the process was stopped by a signal
			if(WIFSIGNALED(test_method) && child_to_check == pid_array[i] && WTERMSIG(test_method) != 11){
				//Print the termination signal
				printf("Backgroud pid %d is done: Terminated by signal %d\n", pid_array[i], WTERMSIG(test_method));
				fflush(stdout);
				//Save the termination signal for use in status
				memset(signal_status, '\0', sizeof(signal_status));
				sprintf(signal_status, "Terminated by signal %d\n", WTERMSIG(test_method));
				//Remove the pid from the array
				remove_pid(pid_array, i);
			}
			//Increment iterator
			i++;
		}


		//Prompt the user for input
		printf(": ");
		fflush(stdout);
		num_characters = getline(&buffer, &bufsize, stdin);
		//If the user entered something, remove the newline character
		if(num_characters != 0){
			buffer[num_characters - 1] = '\0';
		}

		//Put the buffer into an input string
		memset(input, '\0', sizeof(input));
		strcpy(input, buffer);

		//Reset the arg_array by filling with null
		for(i = 0; i < 512; i++){
			arg_array[i] = NULL;
		}

		//If there exists user input
		if(strlen(input) != 0){
			//Start tokenizing with space delimiters
			token = strtok(input, " ");
			arg_array[0] = token;

			//For every possible space in the arg_array
			for(i = 1; i < 512; i++){
				//Take a new token with space delimiters
				token = strtok(NULL, " ");
				//If the new space is not null, then add to the array
				if(token != NULL){
					arg_array[i] = token;
				}
			}
			//Create a pid_buffer string for use in converting $$ to pid in args
			char pid_buffer[100];
			memset(pid_buffer, '\0', sizeof(pid_buffer));

			//For every possible space in the arg_array
			for(i = 0; i < 512; i++){
				//If the argument is not null AND contains $$
				if(arg_array[i] != NULL && strstr(arg_array[i], "$$") != NULL){
					//Get the location of the $$
					char* pid_location = strstr(arg_array[i], "$$");
					//Replace the $$ with %d
					strncpy(pid_location, "%d", 2);
					//Create new string with pid inserted
					sprintf(pid_buffer, arg_array[i], getpid());
					//Replace the arg with the expanded pid
					strcpy(arg_array[i], pid_buffer);
				}
			}
		}

		//If the user inputs a comment with #
		if(buffer[0] == '#'){
			//For every possible arguement space
			for(i = 0; i < 512; i++){
				//Set arguments to null to erase them
				arg_array[i] == NULL;
				if(arg_array[i+1] == NULL){
					break;
				}
			}
		}

		//If the user wants to exit
		else if(arg_array[0] != NULL && !strcmp(arg_array[0], "exit")){
			//For every possible pid in the pid_array
			for(i = 0; i < 512; i++){
				//Terminate the child processes still running in the background
				if(i != 0){
					kill(pid_array[i], SIGTERM);
				}
			}
			//Return the parent to end the program
			return 0;
		}

		//If the user wants to mode directories
		else if(arg_array[0] != NULL && !strcmp(arg_array[0], "cd")){
			//Get the home path
			char* home_path = getenv("HOME");
			//Initalize a cd status
			int cd_result = -5;

			//If cd is the only arguement given
			if(arg_array[1] == NULL){
				//Move to the home path and set a exit status of 0
				cd_result = chdir(home_path);
				exit_status = 0;
			}
			else{
				//If given an arguement, attempt to change to the given directory
				cd_result = chdir(arg_array[1]);
				//If the move failed, print the error and save the status
				if(cd_result == -1){
					perror("cd failure");
					memset(signal_status, '\0', sizeof(signal_status));
					exit_status = 1;
				}
				//If the move was successful, print the error and save the status
				else{
					memset(signal_status, '\0', sizeof(signal_status));
					exit_status = 0;
				}
			}
		}

		//If the user wants to check the status
		else if(arg_array[0] != NULL && !strcmp(arg_array[0], "status")){
			//If there was no signal termination
			if(signal_status[0] == '\0'){
				//Print the int exit value
				printf("exit value %d\n", exit_status);
				fflush(stdout);
			}
			//If there was a signal termination
			else{
				//Print the save termination status
				printf("%s", signal_status);
				fflush(stdout);
			}
		}

		//If user provides a non built-in command
		else{
			//Save the input/output locations for reset later
			int saved_stdout = dup(1);
			int saved_stdin = dup(0);

			//Create and inialize file descriptors for read, write, and dev/null
			int write_fd = -5;
			int read_fd = -5;
			int null_fd = -5;

			//For every possible arguement in the arg_array
			for(i = 1; i < 512; i++){
				//If the current and next arguements are not null, and the current arguement is a ">"
				if(arg_array[i] != NULL && arg_array[i+1] != NULL && !strcmp(arg_array[i], ">")){
					//Open or create the file to the right of the ">" for write only
					write_fd = open(arg_array[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
					//If the file could not be opened
					if(write_fd == -1){
						//Print an error message and save the exit status
						perror("open()");
						exit_status = 1;

						//Remove all arguements relating to the ">"
						remove_element(arg_array, i);
						remove_element(arg_array, i);
						remove_element(arg_array, i);
						//Start searching through args from the beginning
						i = 0;
					}
					//If the file could be opened
					else{
						//Redirect stdout to write to that file
						int dup_result = dup2(write_fd, 1);

						//Remove the ">" and the written to file from the args_array
						remove_element(arg_array, i+1);
						remove_element(arg_array, i+1);
						//Start searching through args from the beginning
						i = 0;
					}
				}

				//If the current and next arguements are not null, and the current arguement is a "<"
				if(arg_array[i] != NULL && arg_array[i+1] != NULL && !strcmp(arg_array[i], "<")){
					//Open the file to the write of the "<" for read only
					read_fd = open(arg_array[i+1], O_RDONLY);
					//If the file could not be opened
					if(read_fd == -1){
						//Print an error message and save the exit status
						perror("open()");
						exit_status = 1;

						//Remove all arguements relating to the "<"
						remove_element(arg_array, i);
						remove_element(arg_array, i);
						remove_element(arg_array, i);
						//Start searching through args from the beginning
						i = 0;
					}
					//If the file could be opened
					else{
						//Redirect stdin to read from the file
						int dup_result = dup2(read_fd, 0);	

						//Remove the "<" and the read from file from the args_array
						remove_element(arg_array, i+1);
						remove_element(arg_array, i+1);
						//Start searching through args from the beginning
						i = 0;
					}

				}
			}
			//Create variables to fork() a new child
			pid_t spawn_pid = -5;
			int child_exit_method = -5;
			//Create a flag int to check if a process should be run in the background
			int bg_check = 0;
			//For every possible arguement in arg_array
			for(i = 0; i < 511; i++){
				//If the last argument is "&"
				if(arg_array[i] != NULL && !strcmp(arg_array[i],"&") && arg_array[i+1] == NULL){
					//Remove the "&"
					remove_element(arg_array, i+1);
					//If foreground-only mode is off
					if(SIGTSTP_flag == 0){
						//Set the flag to put the child in the background
						bg_check = 1;
						//Open dev/null
						null_fd = open("/dev/null", O_RDWR);
						//If the user hasn't specified an input location, set stdin to dev/null
						if(read_fd != 0){
							dup2(null_fd, 0);
						}
						//If the user hasn't specified an output location, set stdout to dev/null
						if(write_fd != 0){
							dup2(null_fd, 1);
						}
					}
					//If foreground-only mode in on, set the flag to put the child in the foreground
					else{
						bg_check = 0;
					}
				}
			}
			//Fork the a new child
			spawn_pid = fork();
			//Print an error if the fork was unsuccessful and exit
			if(spawn_pid == -1){
				perror("Hull Breach!\n");
				exit(1);
			}

			//Childs code
			else if(spawn_pid == 0){
				//If the child is in the foreground, set the SIGINT signal to SIG_DFL
				if(bg_check == 0){
					sigaction(SIGINT, &SIGINT_action, NULL);
				}
				//Exec with the given arguements in arg_array
				if(execvp(*arg_array, arg_array) < 0){
					//Print an error if exec failed
					perror("Exec failure");
					exit(1);
				}
				exit(0);
			}
			//If the child is in the background
			if(bg_check == 1){
				//Reset the I/O to defaults to print the childs pid
				dup2(saved_stdin, 0);
				dup2(saved_stdout, 1);

				printf("Background pid is %d\n", spawn_pid);
				fflush(stdout);

				//Reset I/O to dev/null
				dup2(null_fd, 0);
				dup2(null_fd, 1);

				//Insert the childs pid to the background pid array
				insert_pid(pid_array, spawn_pid);
			}
			//If the child is in the foreground
			if(bg_check == 0){
				//have the parent wait until the child is done
				waitpid(spawn_pid, &child_exit_method, 0);
				//If the child exited with a signal
				if(WIFSIGNALED(child_exit_method) != 0 && WTERMSIG(child_exit_method) != 11){
					//Print the signal that terminated the child to the user and 
					//save the message for use in status
					printf("Terminated by signal %d\n", WTERMSIG(child_exit_method));
					memset(signal_status, '\0', sizeof(signal_status));
					sprintf(signal_status, "Terminated by signal %d\n", WTERMSIG(child_exit_method));
				}
			}

			//Reset the I/O to default
			dup2(saved_stdin, 0);
			dup2(saved_stdout, 1);

			//If the child completed successfully without signals
			if(child_exit_method == 0 && WIFSIGNALED(child_exit_method) == 0){
				//Set the exit status
				memset(signal_status, '\0', sizeof(signal_status));
				exit_status = 0;
			}
			//If the child completed unsuccessfully without signals
			else if(child_exit_method != 0 && WIFSIGNALED(child_exit_method) == 0){
				//Set the exit status
				memset(signal_status, '\0', sizeof(signal_status));
				exit_status = 1;
			}
		}
	}

	return 0;
	}
