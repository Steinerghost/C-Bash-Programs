#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/****GLOBALS*******/
int SIGTSTP_flag = 0;

void remove_element(char** char_array, int index){
	int i;
	for(i = index-1; i < 512; i++){
		if(char_array[i] == NULL){
			break;
		}
		char_array[i] = char_array[i+1];
	}
}

void insert_pid(int* pid_array, int my_pid){
	int i = 0;
	int array_length = 0;
	while(pid_array[i] != 0){
		array_length++;
		i++;
	}
	pid_array[array_length] = my_pid;
}

void remove_pid(int* pid_array, int index){
	int array_length = 0;
	int i = 0;
	while(pid_array[i] != 0){
		array_length++;
		i++;
	}

	for(i = index; i < array_length-1; i++){
		pid_array[i] = pid_array[i+1];
	}
	pid_array[i] = 0;
}

void catchSIGTSTP(int signo){
	char message[500];
	memset(message, '\0', sizeof(message));
	if(SIGTSTP_flag == 0){
		sprintf(message, "\nEntering foreground-only mod (& is now ignored)\n");
		write(STDOUT_FILENO, message, sizeof(message));
		SIGTSTP_flag = 1;
		write(STDOUT_FILENO, ": ", 2);
	}
	else if(SIGTSTP_flag == 1){
		sprintf(message, "\nExiting foreground-only mode\n");
		write(STDOUT_FILENO, message, sizeof(message));
		SIGTSTP_flag = 0;
		write(STDOUT_FILENO, ": ", 2);
	}
	//	char* message = "iCaught SIGTSTP. Test Complete!\n";
	//	write(STDOUT_FILENO, message, 32);
}

int main(){
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};

	ignore_action.sa_handler = SIG_IGN;
	SIGINT_action.sa_handler = SIG_DFL;

	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;

	sigaction(SIGINT, &ignore_action, NULL);

	//	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
	//	sigaction(SIGCHLD, &SIGCHLD_action, NULL);

	char* buffer;
	size_t bufsize = 0;
	int num_characters = -5;
	char* arg_array[512];
	char input[2048];

	int test_method = -5;
	int pid_array[512];

	char* token;
	int i;

	int exit_status = 0;
	char signal_status[300];

	while(1){
		i = 0;
//		memset(signal_status, '\0', sizeof(signal_status));
		while(pid_array[i] != 0){
			test_method = -5;
			int child_to_check = waitpid(pid_array[i], &test_method, WNOHANG);
			if(WIFEXITED(test_method) && child_to_check == pid_array[i]){
				printf("Background pid %d is done: exit value %d\n", pid_array[i], WEXITSTATUS(test_method));
				fflush(stdout);
				remove_pid(pid_array, i);
			}
			if(WIFSIGNALED(test_method) && child_to_check == pid_array[i] && WTERMSIG(test_method) != 11){
				printf("Backgroud pid %d is done: Terminated by signal %d\n", pid_array[i], WTERMSIG(test_method));
				fflush(stdout);
				memset(signal_status, '\0', sizeof(signal_status));
				sprintf(signal_status, "Terminated by signal %d\n", WTERMSIG(test_method));
			}
			i++;
		}


		printf(": ");
		fflush(stdout);
		num_characters = getline(&buffer, &bufsize, stdin);
		if(num_characters != 0){
			buffer[num_characters - 1] = '\0';
		}

		memset(input, '\0', sizeof(input));
		strcpy(input, buffer);
		for(i = 0; i < 512; i++){
			arg_array[i] = NULL;
		}

		if(strlen(input) != 0){
			token = strtok(input, " ");

			arg_array[0] = token;
			if(!strcmp(arg_array[0], "$$")){
				sprintf(arg_array[0], "%d", getpid());
			}
			for(i = 1; i < 512; i++){
				token = strtok(NULL, " ");
				if(token != NULL){
					arg_array[i] = token;
				}
			}
			char pid_buffer[100];
			memset(pid_buffer, '\0', sizeof(pid_buffer));

			for(i = 0; i < 512; i++){
				if(arg_array[i] != NULL && strstr(arg_array[i], "$$") != NULL){
					char* pid_location = strstr(arg_array[i], "$$");
					strncpy(pid_location, "%d", 2);
					sprintf(pid_buffer, arg_array[i], getpid());
					strcpy(arg_array[i], pid_buffer);
				}
			}
		}
		if(buffer[0] == '#'){
			//			printf("%s\n", buffer);
			//			fflush(stdout);
			for(i = 0; i < 512; i++){
				arg_array[i] == NULL;
				if(arg_array[i+1] == NULL){
					break;
				}
			}
		}

		else if(arg_array[0] != NULL && !strcmp(arg_array[0], "exit")){
			for(i = 0; i < 512; i++){
				if(i != 0){
					kill(pid_array[i], SIGTERM);
				}
			}
			return 0;
		}

		else if(arg_array[0] != NULL && !strcmp(arg_array[0], "cd")){
			char* home_path = getenv("HOME");
			int cd_result = -5;
			if(arg_array[1] == NULL){
				cd_result = chdir(home_path);
				exit_status = 0;
			}
			else{
				cd_result = chdir(arg_array[1]);
				if(cd_result == -1){
					perror("cd failure");
					memset(signal_status, '\0', sizeof(signal_status));
					exit_status = 1;
				}
				else{
					memset(signal_status, '\0', sizeof(signal_status));
					exit_status = 0;
				}
			}
		}

		else if(arg_array[0] != NULL && !strcmp(arg_array[0], "status")){
			if(signal_status[0] == '\0'){
				printf("exit value %d\n", exit_status);
				fflush(stdout);
			}
			else{
				printf("%s", signal_status);
				fflush(stdout);
			}
		}

		//		else if(arg_array[0] != NULL && (!strcmp(arg_array[0], "echo") || !strcmp(arg_array[0], "ls") || !strcmp(arg_array[0], "cat"))){
		else{
			int saved_stdout = dup(1);
			int saved_stdin = dup(0);
			int write_fd = -5;
			int read_fd = -5;
			int null_fd = -5;
			for(i = 1; i < 512; i++){
				if(arg_array[i] != NULL && arg_array[i+1] != NULL && !strcmp(arg_array[i], ">")){
					write_fd = open(arg_array[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if(write_fd == -1){
						perror("open()");
						exit_status = 1;

						remove_element(arg_array, i);
						remove_element(arg_array, i);
						remove_element(arg_array, i);
						//						arg_array[i-1] = NULL;
						//						arg_array[i] = NULL;
						//						arg_array[i+1] = NULL;
						i = 0;
					}
					else{
						int dup_result = dup2(write_fd, 1);

						remove_element(arg_array, i+1);
						remove_element(arg_array, i+1);
						//						arg_array[i] = NULL;
						//						arg_array[i+1] = NULL;
						i = 0;
					}
				}
				if(arg_array[i] != NULL && arg_array[i+1] != NULL && !strcmp(arg_array[i], "<")){
					read_fd = open(arg_array[i+1], O_RDONLY);
					if(read_fd == -1){
						perror("open()");
						exit_status = 1;
						remove_element(arg_array, i);
						remove_element(arg_array, i);
						remove_element(arg_array, i);

						//						arg_array[i-1] = NULL;	
						//				arg_array[i] = NULL;
						//						arg_array[i+1] = NULL;		
						i = 0;

					}
					else{
						int dup_result = dup2(read_fd, 0);	
						remove_element(arg_array, i+1);
						remove_element(arg_array, i+1);
						//						arg_array[i] = NULL;
						//						arg_array[i+1] = NULL;		
						i = 0;

					}

				}
			}
			pid_t spawn_pid = -5;
			int child_exit_method = -5;
			int bg_check = 0;
			for(i = 0; i < 511; i++){
				if(arg_array[i] != NULL && !strcmp(arg_array[i],"&") && arg_array[i+1] == NULL){
					remove_element(arg_array, i+1);
					if(SIGTSTP_flag == 0){
						bg_check = 1;
						null_fd = open("/dev/null", O_RDWR);
						if(read_fd != 0){
							dup2(null_fd, 0);
						}
						if(write_fd != 0){
							dup2(null_fd, 1);
						}
					}
					else{
						bg_check = 0;
					}
				}
			}
			spawn_pid = fork();
			if(spawn_pid == -1){
				perror("Hull Breach!\n");
				exit(1);
			}
			else if(spawn_pid == 0){
				if(bg_check == 0){
					sigaction(SIGINT, &SIGINT_action, NULL);
				}
				if(execvp(*arg_array, arg_array) < 0){
					perror("Exec failure");
					exit(1);
				}
				exit(0);
			}
			/**********************************************
			 * check on this MORE HOW TO GET ZOMBIES!?
			 * *********************************************/
			if(bg_check == 1){
				dup2(saved_stdin, 0);
				dup2(saved_stdout, 1);

				printf("Background pid is %d\n", spawn_pid);
				fflush(stdout);

				dup2(null_fd, 0);
				dup2(null_fd, 1);


				insert_pid(pid_array, spawn_pid);

				//		waitpid(spawn_pid, &child_exit_method, WNOHANG);
			}
			if(bg_check == 0){
				waitpid(spawn_pid, &child_exit_method, 0);
				if(WIFSIGNALED(child_exit_method) != 0 && WTERMSIG(child_exit_method) != 11){
					printf("Terminated by signal %d\n", WTERMSIG(child_exit_method));
					memset(signal_status, '\0', sizeof(signal_status));
					sprintf(signal_status, "Terminated by signal %d\n", WTERMSIG(child_exit_method));
				}
			}

			dup2(saved_stdin, 0);
			dup2(saved_stdout, 1);

			if(child_exit_method == 0 && WIFSIGNALED(child_exit_method) == 0){
				memset(signal_status, '\0', sizeof(signal_status));
				exit_status = 0;
			}
			else if(child_exit_method != 0 && WIFSIGNALED(child_exit_method) == 0){
				memset(signal_status, '\0', sizeof(signal_status));
				exit_status = 1;
			}
		}
	}

	return 0;
	}
