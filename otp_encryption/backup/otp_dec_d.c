#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg){
	perror(msg);
	exit(0);
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

int main(int argc, char* argv[]){
	int listen_socket_FD, established_connection_FD, port_number, chars_read;
	socklen_t sizeof_client_info;
	char buffer[1000000];
	char current_buffer[1000];

	struct sockaddr_in server_address, client_address;

	if(argc < 2){
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	}

	memset((char *)&server_address, '\0', sizeof(server_address));
	port_number = atoi(argv[1]);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_number);
	server_address.sin_addr.s_addr = INADDR_ANY;

	listen_socket_FD = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_socket_FD < 0){
		error("ERROR opening socket");
	}

	if(bind(listen_socket_FD, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
		error("ERROR on binding");
	}
	listen(listen_socket_FD, 5);

	int pid_array[1000];
	memset(pid_array, 0, sizeof(pid_array));
	int test_method = -5;
	while(1){
		sizeof_client_info = sizeof(client_address);
		established_connection_FD = accept(listen_socket_FD, (struct sockaddr *)&client_address, &sizeof_client_info);
		if(established_connection_FD < 0){
			error("ERROR on accept");
		}

//		printf("SERVER otp_enc_d: Connected client at port %d\n", ntohs(client_address.sin_port));

		pid_t spawn_pid = -5;
		int child_exit_method = -5;
		spawn_pid = fork();
		if(spawn_pid == -1){
			perror("Hull Breach!\n");
			exit(1);
		}

		else if(spawn_pid == 0){
			chars_read = send(established_connection_FD, "otp_dec_d", 10, 0);
			
			memset(buffer, '\0', sizeof(buffer));

			while(strstr(buffer, "@@") == NULL){
				memset(current_buffer, '\0', sizeof(current_buffer));
				chars_read = recv(established_connection_FD, current_buffer, sizeof(current_buffer)-1, 0);
				strcat(buffer, current_buffer);
				if(chars_read == -1 || chars_read == 0){
					break;	
				}
			}
			if(chars_read < 0){
				error("ERROR writing to socket");
			}
			char* token = strtok(buffer, "/");

			if(token == "false"){
				exit(1);
			}

			token = strtok(NULL, "/");
//			printf("SERVER otp_dec_d: message to be decrypted: %s\n", token);
			//KEEP IN MIND CHAR LIMITS. MIGHT BE HARD TO DEBUG!
			char dec_message[1000000];
			memset(dec_message, '\0', sizeof(dec_message));
		  	strcpy(dec_message, token);

			token = strtok(NULL, "/");
//			printf("SERVER otp_dec_d: key for decryption: %s\n", token);
			char dec_key[1000000];
		   memset(dec_key, '\0', sizeof(dec_key));
			strcpy(dec_key, token);

			int i;
			char dec_answer[1000000];
			memset(dec_answer, '\0', sizeof(dec_answer));
			while(dec_message[i] != '\0'){
				if(dec_message[i] == 32){
					dec_message[i] = 64;
				}
				if(dec_key[i] == 32){
					dec_key[i] = 64;
				}
				dec_answer[i] = (dec_message[i] - dec_key[i]) % 27;
			  	if((int)dec_answer[i] < 0){
					dec_answer[i] = dec_answer[i] + 64 + 27;
				}
				else{
					dec_answer[i] = dec_answer[i] + 64;
				}
				if(dec_answer[i] == 64){
					dec_answer[i] = 32;
				}
//				printf("%c", enc_answer[i]);
				i++;
			}
//			printf("\n");

			dec_answer[i] = '\n';
			strcat(dec_answer, "@@");
			chars_read = send(established_connection_FD, dec_answer, 1000000, 0);

			close(established_connection_FD);
			exit(0);
		}
		do{
			waitpid(-1, &test_method, WNOHANG);
		}while(test_method > 0);
//		close(established_connection_FD);
	}
	close(listen_socket_FD);

	return 0;
}
