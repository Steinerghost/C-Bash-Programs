#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg){
	perror(msg);
	exit(0);
}

int main(int argc, char* argv[]){
	int socket_FD, port_number, chars_written, chars_read;
	struct sockaddr_in server_address;
	struct hostent* server_host_info;
	char buffer[100000];

	if(argc < 3){
		fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
		exit(0);
	}

	memset((char*)&server_address, '\0', sizeof(server_address));
	port_number = atoi(argv[3]);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_number);
	server_host_info = gethostbyname("localhost");
	if(server_host_info == NULL){
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	memcpy((char*)&server_address.sin_addr.s_addr, (char*)server_host_info->h_addr, server_host_info->h_length);

	socket_FD = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_FD < 0){
		error("CLIENT: ERROR opening socket");
	}

	if(connect(socket_FD, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
		error("CLIENT: ERROR connecting");
	}

//	printf("CLIENT: Enter text to send to the server, and then hit enter: ");
//	memset(buffer, '\0', sizeof(buffer));
//	fgets(buffer, sizeof(buffer - 1), stdin);
//	buffer[strcspn(buffer, "\n")] = '\0';

	int plaintext_FD = open(argv[1], O_RDONLY);
	ssize_t nread;
	memset(buffer, '\0', sizeof(buffer));
	nread = read(plaintext_FD, buffer, sizeof(buffer));
	char plaintext[100000];	
	buffer[strcspn(buffer, "\n")] = '\0';
  	strcpy(plaintext, buffer);
	close(plaintext_FD);

	char char_list[27];
	char_list[0] = 32; //space
	int i;
	for(i = 1; i < 27; i++){
		char_list[i] = i + 64;
	}	

	for(i = 0; i < strlen(plaintext); i++){
		if(((int)plaintext[i] < 65 || (int)plaintext[i] > 90) && (int)plaintext[i] != 32){
			fprintf(stderr, "Plaintext file contains bad characters.\n");
			exit(1);
		}
	}

	char send_text[200000];

	memset(buffer, '\0', sizeof(buffer));
	chars_read = recv(socket_FD, buffer, sizeof(buffer) -1, 0);
	if(chars_read < 0){
		error("CLIENT: ERROR reading from scoket");
	}
//	printf("CLIENT: I recieved this from the server: \"%s\"\n", buffer);
	char input[100];
	strcpy(input, buffer);
	if(strcmp(input, "otp_enc_d")){
		fprintf(stderr, "Mismatch of encoding/decoding servers.\n");
		strcpy(send_text, "false/");
		exit(2);
	}
	strcpy(send_text, "true/");

	memset(buffer, '\0', sizeof(buffer));
	int key_FD = open(argv[2], O_RDONLY);
	nread = read(key_FD, buffer, sizeof(buffer));
	char keytext[100000];
	buffer[strcspn(buffer, "\n")] = '\0';
	strcpy(keytext, buffer);
	close(key_FD);
	
	if(strlen(keytext) < strlen(plaintext)){
		fprintf(stderr, "Key %s is too short.\n", argv[2]);
		exit(1);
	}

	strcat(send_text, plaintext);
	strcat(send_text, "/");
	strcat(send_text, keytext);
	strcat(send_text, "@@");   //Terminate message

//	printf("sending message: %s\n", send_text);

	chars_written = send(socket_FD, send_text, strlen(send_text), 0);
	if(chars_written < 0){
		error("CLIENT: ERROR writing to socket");
	}
	if(chars_written < strlen(buffer)){
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	}

	memset(buffer, '\0', sizeof(buffer));
	char current_buffer[1000];
	while(strstr(buffer, "@@") == NULL){
		memset(current_buffer, '\0', sizeof(current_buffer));
		chars_written = recv(socket_FD, current_buffer, sizeof(current_buffer)-1, 0);
		strcat(buffer, current_buffer);
		if(chars_read == -1 || chars_read == 0){
			break;
		}
	}
	if(chars_written < 0){
		error("CLIENT ERROR reading encoded message");
	}

	buffer[strcspn(buffer, "@")] = '\0';
	
	char enc_message[100000];
	strcpy(enc_message, buffer);
	printf("%s", enc_message);

	close(socket_FD);

	return 0;
}
