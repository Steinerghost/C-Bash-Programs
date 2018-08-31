/**************************************************************************************
 * Author: Casey Ford
 * Date: 6/10/18
 ***************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//Function to handle error messages, passes msg to perror to print extra info
//then exits
void error(const char *msg){
	perror(msg);
	exit(0);
}

int main(int argc, char* argv[]){
	//Define variables to store socket information
	int socket_FD, port_number, chars_written, chars_read;
	struct sockaddr_in server_address;
	struct hostent* server_host_info;

	//Define buffer variable to store string I/O
	char buffer[1000000];

	//If the number of arguements given is less than 3, print usage error and exit
	if(argc < 3){
		fprintf(stderr, "USAGE: %s ciphertext key port\n", argv[0]);
		exit(0);
	}

	//Define the server_address struct with given user arguements
	memset((char*)&server_address, '\0', sizeof(server_address));
	port_number = atoi(argv[3]);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_number);

	//Connect to localhost server
	server_host_info = gethostbyname("localhost");
	//Print error if connection failed
	if(server_host_info == NULL){
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	//Load host info innto the server_address struct
	memcpy((char*)&server_address.sin_addr.s_addr, (char*)server_host_info->h_addr, server_host_info->h_length);

	//Create a socket to connect to decoding server
	socket_FD = socket(AF_INET, SOCK_STREAM, 0);
	//Print an error if the socket failed to initalize
	if(socket_FD < 0){
		error("CLIENT: ERROR opening socket");
	}
	//Print an error if the socket failed to connect to server
	if(connect(socket_FD, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
		error("CLIENT: ERROR connecting");
	}

	//Open the filename provided by the user for use in decryption
	int ciphertext_FD = open(argv[1], O_RDONLY);

	//Read the contents of the file into the buffer
	ssize_t nread;
	memset(buffer, '\0', sizeof(buffer));
	nread = read(ciphertext_FD, buffer, sizeof(buffer));

	//Copy the info into a newly created char array and then close the file
	char ciphertext[100000];	
	buffer[strcspn(buffer, "\n")] = '\0';
  	strcpy(ciphertext, buffer);
	close(ciphertext_FD);

	//Search the char array for characters other than A-Z and space
	int i;
	//For every char in the array
	for(i = 0; i < strlen(ciphertext); i++){
		//If the char ASCII is not A-Z or a space, print an error and exit
		if(((int)ciphertext[i] < 65 || (int)ciphertext[i] > 90) && (int)ciphertext[i] != 32){
			fprintf(stderr, "Plaintext file contains bad characters.\n");
			exit(1);
		}
	}

	//Initalize a char array to send info to the decryption server
	char send_text[200000];

	//Recieve server name to confirm it's a decryption server
	memset(buffer, '\0', sizeof(buffer));
	chars_read = recv(socket_FD, buffer, sizeof(buffer) -1, 0);
	//Print an error if the recv failed
	if(chars_read < 0){
		error("CLIENT: ERROR reading from scoket");
	}

	//Check the input recieved from the server
	char input[100];
	strcpy(input, buffer);
	//If the server name is not "otp_dec_d"
	if(strcmp(input, "otp_dec_d")){
		//Print a mismatch error and tag the sending information with false
		fprintf(stderr, "Mismatch of encoding/decoding servers.\n");
		strcpy(send_text, "false/");
		//Exit with status 2
		exit(2);
	}
	//If the server did reply with "otp_dec_d", tag the sending information with true
	strcpy(send_text, "true/");

	//Open the key file provided by the user, and put its contents
	//into a new char array called keytext
	memset(buffer, '\0', sizeof(buffer));
	int key_FD = open(argv[2], O_RDONLY);
	nread = read(key_FD, buffer, sizeof(buffer));
	char keytext[100000];
	//Remove the ending newline with a null terminator
	buffer[strcspn(buffer, "\n")] = '\0';
	strcpy(keytext, buffer);
	//Close the key file
	close(key_FD);
	
	//If the key size if less than the ciphertext size, print an error
	if(strlen(keytext) < strlen(ciphertext)){
		fprintf(stderr, "Key %s is too short.\n", argv[2]);
		exit(1);
	}

	//Compile the text being sent to the decryption server
	strcat(send_text, ciphertext);
	strcat(send_text, "/");       //Message parts are "/" delimited
	strcat(send_text, keytext);
	strcat(send_text, "@@");      //Terminating characters

	//Send the text to the encryption server
	chars_written = send(socket_FD, send_text, strlen(send_text), 0);
	//Print an error if send failed
	if(chars_written < 0){
		error("CLIENT: ERROR writing to socket");
	}
	//Print a warning if not all info was sent
	if(chars_written < strlen(buffer)){
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	}

	//Prepare to recieve decoded message
	memset(buffer, '\0', sizeof(buffer));
	char current_buffer[1000];
	//While the incoming stream of bits hasn't recieved a string terminating "@@"
	while(strstr(buffer, "@@") == NULL){
		//Clear the current_buffer and recv data until full
		memset(current_buffer, '\0', sizeof(current_buffer));
		chars_written = recv(socket_FD, current_buffer, sizeof(current_buffer)-1, 0);
		//Add the current buffer to the total buffer
		strcat(buffer, current_buffer);
		//If an error occured, or no more information is avaliable, break out of loop
		if(chars_read == -1 || chars_read == 0){
			break;
		}
	}
	//If an error occured in recieving information, print error
	if(chars_written < 0){
		error("CLIENT ERROR reading decoded message");
	}
	
	//Replace the "@@" terminating characters with null terminators
	buffer[strcspn(buffer, "@")] = '\0';

	//Print the decrypted message to stdout
	char dec_message[100000];
	strcpy(dec_message, buffer);
	printf("%s", dec_message);

	//Close the socket and return
	close(socket_FD);
	return 0;
}
