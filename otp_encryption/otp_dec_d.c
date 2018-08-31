/***********************************************************************************
 * Author: Casey Ford
 * Date: 6/10/18
 **********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Function to handle error messages. Passes msg to perror to print extra info
//then exits
void error(const char *msg){
	perror(msg);
	exit(0);
}

int main(int argc, char* argv[]){
	//Define variables to store socket information
	int listen_socket_FD, established_connection_FD, port_number, chars_read;
	socklen_t sizeof_client_info;
	struct sockaddr_in server_address, client_address;
	
	//Define buffer variables to store I/O strings
	char buffer[1000000];
	char current_buffer[1000];

	//Check the number of arguements. Print the usage of this program if 3 or greater
	if(argc < 2){
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	}

	//Define the server_address struct with the given user arguement
	memset((char *)&server_address, '\0', sizeof(server_address));
	port_number = atoi(argv[1]);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_number);
	server_address.sin_addr.s_addr = INADDR_ANY;

	//Create a socket to listen for incoming transmissions
	listen_socket_FD = socket(AF_INET, SOCK_STREAM, 0);
	//Print an error if the socket failed to initalize
	if(listen_socket_FD < 0){
		error("ERROR opening socket");
	}

	//Bind the socket to the server_address struct. Print an error if failed
	if(bind(listen_socket_FD, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
		error("ERROR on binding");
	}
	//Listen and queue up to 5 incoming transmissions
	listen(listen_socket_FD, 5);

	//Permenently listen for transmissions
	while(1){
		//Accept incoming transmissions that match the port
		sizeof_client_info = sizeof(client_address);
		established_connection_FD = accept(listen_socket_FD, (struct sockaddr *)&client_address, &sizeof_client_info);
		//Print an error if the connection failed
		if(established_connection_FD < 0){
			error("ERROR on accept");
		}

		//Fork a child process to handle the encryption from the newly accepted port
		pid_t spawn_pid = -5;
		int child_exit_method = -5;
		spawn_pid = fork();
		//Print an error if spawning the child failed
		if(spawn_pid == -1){
			perror("Hull Breach!\n");
			exit(1);
		}

		//If spawning was successful, the child code executes here
		else if(spawn_pid == 0){
			//Send a signal to the requesting program telling them the current server
			chars_read = send(established_connection_FD, "otp_dec_d", 10, 0);
			
			//Prepare the buffer to recieve incoming information
			memset(buffer, '\0', sizeof(buffer));

			//While the incoming stream of bits hasn't recieved a string terminating "@@"
			while(strstr(buffer, "@@") == NULL){
				//Clear the current_buffer and recv data until full
				memset(current_buffer, '\0', sizeof(current_buffer));
				chars_read = recv(established_connection_FD, current_buffer, sizeof(current_buffer)-1, 0);
				//Add the current buffer to the total buffer
				strcat(buffer, current_buffer);
				//If an error occured, or no more information is avaliable, break out of loop
				if(chars_read == -1 || chars_read == 0){
					break;	
				}
			}
			//If an error occured in recieving information, print error
			if(chars_read < 0){
				error("ERROR writing to socket");
			}
			//Start separating the "/" delimited message
			char* token = strtok(buffer, "/");
			//If the first part of the message is false, the servers are missmatched.
			//Exit out of child
			if(token == "false"){
				exit(1);
			}

			//Put the next part of the input into the dec_message to be decoded
			token = strtok(NULL, "/");
			char dec_message[1000000];
			memset(dec_message, '\0', sizeof(dec_message));
		  	strcpy(dec_message, token);

			//Put the next part of the input into dec_key for use as the decoding key
			token = strtok(NULL, "/");
			char dec_key[1000000];
		   memset(dec_key, '\0', sizeof(dec_key));
			strcpy(dec_key, token);

			//Initalize variables for decoding
			int i;
			char dec_answer[1000000];
			memset(dec_answer, '\0', sizeof(dec_answer));
			//While the message has not been completely decoded
			while(dec_message[i] != '\0'){
				//If the char in the message is a " ", replace with a "@" for OTP
				if(dec_message[i] == 32){
					dec_message[i] = 64;
				}
				//If the char in the key is a " ", replace with a "@" for OTP
				if(dec_key[i] == 32){
					dec_key[i] = 64;
				}
				//Perform the OTP decryption by subtracting the key from the message, and mod 27
				dec_answer[i] = (dec_message[i] - dec_key[i]) % 27;
				//If the answer above was negative, add 27 to account for negative mod
				//and reshift by adding 64
			  	if((int)dec_answer[i] < 0){
					dec_answer[i] = dec_answer[i] + 64 + 27;
				}
				//If the answer wasn't negative, just reshift by adding 64
				else{
					dec_answer[i] = dec_answer[i] + 64;
				}
				//If the answer char is "@" replace with a " ".
				if(dec_answer[i] == 64){
					dec_answer[i] = 32;
				}
				//Increment index
				i++;
			}
	
			//End the decoded answer with a newline and a string terminating "@@"
			dec_answer[i] = '\n';
			strcat(dec_answer, "@@");
			//Send decoded message back to requester
			chars_read = send(established_connection_FD, dec_answer, 1000000, 0);

			//Close the connection to requester
			close(established_connection_FD);
			
			//Exit out of child
			exit(0);
		}
		//Wait on any children that need reaping
		do{
			waitpid(-1, &child_exit_method, WNOHANG);
		}while(child_exit_method > 0);
	}
	//Close the listening socket and return to end
	close(listen_socket_FD);
	return 0;
}
