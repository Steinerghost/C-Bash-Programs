#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;


struct room{
	int num_connections;
	char* name;
	char* type;

	char connection[6][9];
};

char* get_newest_dir(){
	char target_dir_prefix[32] = "fordc.rooms.";
	char* newest_dir_name = malloc(sizeof(char)*50);
	memset(newest_dir_name, '\0', sizeof(newest_dir_name));

	DIR* dir_to_check;
	struct dirent *file_in_dir;
	struct stat dir_attributes;
	int newest_dir_time = 0;

	dir_to_check = opendir(".");

	if(dir_to_check > 0){
		while((file_in_dir = readdir(dir_to_check)) != NULL){
			if(strstr(file_in_dir->d_name, target_dir_prefix) != NULL){
				stat(file_in_dir->d_name, &dir_attributes);

				if((int)dir_attributes.st_mtime > newest_dir_time){
					newest_dir_time= (int)dir_attributes.st_mtime;
					memset(newest_dir_name, '\0', sizeof(newest_dir_name));
					strcpy(newest_dir_name, file_in_dir->d_name);
				}
			}
		}
	}

	closedir(dir_to_check);

	return newest_dir_name;
}

struct room* create_empty_array(){
	struct room* my_rooms = malloc(sizeof(struct room)*7);
	
	int i, j;
	for(i = 0; i < 7; i++){
		my_rooms[i].name = NULL;
		my_rooms[i].type = NULL;
		my_rooms[i].num_connections = 0;

		for(j = 0; j < 6; j++){
			memset(my_rooms[i].connection[j], '\0', sizeof(my_rooms[i].connection[j]));
		}
	}

	return my_rooms;
}

void print_location(struct room A){
	printf("CURRENT LOCATION: %s\n", A.name);
	printf("POSSIBLE CONNECTIONS: ");

	int i;
	for(i = 0; i < A.num_connections; i++){
		if(i != A.num_connections - 1){
			printf("%s, ", A.connection[i]);
		}
		else{
			printf("%s.\n", A.connection[i]);
		}
	}
	printf("WHERE TO? >");
}





int is_same_room(struct room A, struct room B){
	if(!strcmp(A.name, B.name)){
		return 1;
	}
	return 0;
}

void* running_time(void* input){
	pthread_mutex_lock(&my_mutex);

	char the_time[100];
	memset(the_time, '\0', sizeof(the_time));

	time_t mytime;
	struct tm info;

	time(&mytime);
	info = *localtime(&mytime);
	strftime(the_time, sizeof(the_time), "%l:%M%P, %A, %B %d, %Y", &info);

	int file_descriptor;
	file_descriptor = open("currentTime.txt", O_RDWR | O_CREAT, 0777);

	ssize_t nwritten;
	nwritten = write(file_descriptor, the_time, strlen(the_time)*sizeof(char));

	pthread_mutex_unlock(&my_mutex);
	return NULL;
}

void get_time(){
//	do{
//	if(lock_fd == -1){
//		if(errno == EEXIST){
//			sleep(1);
//		}
//		else{
//			perror("Couldn't open lock file\n");
//			exit(-1);
//		}
//	}
///
//	}while(lock_fd = -1);
	

	pthread_mutex_unlock(&my_mutex);
	sleep(1);
	char read_buffer[1000];
	memset(read_buffer, '\0', sizeof(read_buffer));

	ssize_t nread;
	int file_descriptor;
	file_descriptor = open("currentTime.txt", O_RDWR | O_CREAT, 0777);

	lseek(file_descriptor, 0, SEEK_SET);
	nread = read(file_descriptor, read_buffer, sizeof(read_buffer));
	
	printf("\n%s\n", read_buffer);
	

	pthread_mutex_lock(&my_mutex);

	int result_int, index;
	pthread_t my_thread_ID;
//	int thread_args;

	result_int = pthread_create(&my_thread_ID, NULL, running_time, NULL);

}
struct room get_new_location(struct room* my_rooms, struct room current_room){
	char* buffer = NULL;
	size_t buffer_size = 0;
	int num_chars_entered = -5;

	struct room new_room;
	char* new_room_name = NULL;

	num_chars_entered = getline(&buffer, &buffer_size, stdin);
	buffer[num_chars_entered - 1] = '\0';

	int i, j;

	for(i = 0; i < current_room.num_connections; i++){
		if(!strcmp(buffer, current_room.connection[i])){
			new_room_name = current_room.connection[i];
		}
	}

	if(new_room_name != NULL){
		for(i = 0; i < 7; i++){
			if(!strcmp(new_room_name, my_rooms[i].name)){
				new_room = my_rooms[i];
				return new_room;
			}
		}
	}
	else if(!strcmp(buffer, "time")){
		get_time();
		return current_room;
	}
	else{
		printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
		return current_room;
	}
}
int main(){
	pthread_mutex_lock(&my_mutex);

	/***************************************************/
	int result_int, index;
	pthread_t my_thread_ID;
//	int thread_args;

	result_int = pthread_create(&my_thread_ID, NULL, running_time, NULL);
/*******************************************************/

	struct room* my_rooms = create_empty_array();
	char* dir_name = get_newest_dir();
//	printf("newest directory: %s\n", dir_name);


	DIR* dir_to_check = opendir(dir_name);
	int i = 0;
	int j = 0;
	int file_descriptor;
	struct dirent *file_in_dir;
	ssize_t nread;
	char read_buffer[1000];
	char file_path[1000];
	char* token;
	while((file_in_dir = readdir(dir_to_check)) != NULL){
		token = NULL;
		if(strcmp(file_in_dir->d_name, ".") != 0 && strcmp(file_in_dir->d_name, "..") != 0){
//			printf("FILE %d is named: %s\n", i+1, file_in_dir->d_name);

			memset(file_path, '\0', sizeof(file_path));
			sprintf(file_path, "%s/%s", dir_name, file_in_dir->d_name);

			file_descriptor = open(file_path, O_RDONLY);
			if(file_descriptor == -1){
				printf("Hull breach - open() failed on \"%s\"\n", file_path);
			}



			memset(read_buffer, '\0', sizeof(read_buffer));
			lseek(file_descriptor, 0, SEEK_SET);
			nread = read(file_descriptor, read_buffer, sizeof(read_buffer));

//			printf("File contents:\n%s", read_buffer);

			//Get Name
			token = strtok(read_buffer, " \n");
			token = strtok(NULL, " \n");
			token = strtok(NULL, " \n");

			my_rooms[i].name = strdup(token);

			//Get Connections
			do{
				token = strtok(NULL, " \n");
				token = strtok(NULL, " \n");
				token = strtok(NULL, " \n");
//				printf("Token currently at: %s\n", token);


				if(strcmp(token, "MID_ROOM") && strcmp(token, "START_ROOM") && strcmp(token, "END_ROOM")){
					strcpy(my_rooms[i].connection[j], strdup(token));
				}
				j++;
			}while(strcmp(token, "MID_ROOM") && strcmp(token, "START_ROOM") && strcmp(token, "END_ROOM"));
			my_rooms[i].num_connections = j - 1;
			j = 0;

			//Get type
			my_rooms[i].type = strdup(token);

			i++;
//			printf("\n");
			close(file_descriptor);
		}
	}
	closedir(dir_to_check);

//	printf("\n");
//	for(i = 0; i < 7; i++){
//		printf("ROOM %d\n", i+1);
//		printf("Name: %s\n", my_rooms[i].name);
//		printf("Number of connections: %d\n", my_rooms[i].num_connections);
//		for(j = 0; j < my_rooms[i].num_connections; j++){
//			printf("Connection %d: %s\n", j+1, my_rooms[i].connection[j]);
//		}
//		printf("Type: %s\n", my_rooms[i].type);
//	}

	//PLAY THE GAME
	struct room current_room;
	struct room new_room;
	int num_steps = 0;
	char* room_path = NULL;

	for(i = 0; i < 7; i++){
		if(!strcmp(my_rooms[i].type, "START_ROOM")){
			current_room = my_rooms[i];
		}
	}
	while(strcmp(current_room.type, "END_ROOM")){
		print_location(current_room);	
		new_room = get_new_location(my_rooms, current_room);
		if(!is_same_room(new_room, current_room)){
			num_steps++;
			if(num_steps == 1){
				room_path = strdup(new_room.name);
				strcat(room_path, "\n");
			}
			else{
				strcat(room_path, new_room.name);
				strcat(room_path, "\n");
			}
			current_room = new_room;
		}
		printf("\n");
	}

	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n%s", num_steps, room_path);
/***************************************************/
//	result_int = pthread_join(my_thread_ID, NULL);
//	printf("Thread is dead...\n");
/*******************************************************/
	pthread_mutex_destroy(&my_mutex);
	free(my_rooms);
	free(dir_name);
	return 0;
}
