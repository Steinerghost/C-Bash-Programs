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

//Create a global mutex to handle read/write access to the currentTime/txt file
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

//The room struct object to contain room information
struct room{
	int num_connections;        //The number of connections to other rooms
	char* name;                 //The name of the room
	char* type;                 //The type of the room (START_ROOM/MID_ROOM/END_ROOM)

	//An array of names (max of 6 with max name length of 8)
	char connection[6][9];
};

//Determines the newest modified directory and returns its name
char* get_newest_dir(){

	//Store the room prefix to enable searching by pID suffix
	char target_dir_prefix[32] = "fordc.rooms.";

	//Create space for the newest dir name and clear our the contents with '\0'
	char* newest_dir_name = malloc(sizeof(char)*50);
	memset(newest_dir_name, '\0', sizeof(newest_dir_name));

	//Define variables for use in directory search
	DIR* dir_to_check;             //Holds current directory
	struct dirent *file_in_dir;    //Holds the sub directory within the current directory
	struct stat dir_attributes;    //Holds that attributes of the sub directory
	int newest_dir_time = 0;       //The timestamp of the newest subdirectory

	//Open the durrent directory
	dir_to_check = opendir(".");

	//Confirm the current directory was opened
	if(dir_to_check > 0){
		//Check all sub directories
		while((file_in_dir = readdir(dir_to_check)) != NULL){
			//If the directory has the correct prefix defined above
			if(strstr(file_in_dir->d_name, target_dir_prefix) != NULL){
				//Get the attributes of that subdirectory
				stat(file_in_dir->d_name, &dir_attributes);

				//If the subdirectory time is bigger than the previous
				if((int)dir_attributes.st_mtime > newest_dir_time){

					//Set the highest time as the newest_dir_time and then
					//set the newest directory name to the one with the highest time
					newest_dir_time= (int)dir_attributes.st_mtime;
					memset(newest_dir_name, '\0', sizeof(newest_dir_name));
					strcpy(newest_dir_name, file_in_dir->d_name);
				}
			}
		}
	}

	//Close the current directory
	closedir(dir_to_check);

	//Return the newest directory name
	return newest_dir_name;
}

//Creates and initalizes an empty room array of size 7 and returns the array pointer
struct room* create_empty_array(){
	//Malloc enough space for the 7 rooms
	struct room* my_rooms = malloc(sizeof(struct room)*7);
	
	int i, j;
	//For every room in the array
	for(i = 0; i < 7; i++){
		//Set all strings to NULL and all ints to 0
		my_rooms[i].name = NULL;
		my_rooms[i].type = NULL;
		my_rooms[i].num_connections = 0;

		//For each possible connection (max of 6) empty garbage by setting all slots to '\0'
		for(j = 0; j < 6; j++){
			memset(my_rooms[i].connection[j], '\0', sizeof(my_rooms[i].connection[j]));
		}
	}

	//Return the empty room array
	return my_rooms;
}

//Prints the current location to the player given a room, and gives possible locations
//to travel to next
void print_location(struct room A){
	//Print where the player is currently located
	printf("CURRENT LOCATION: %s\n", A.name);
	//Print where the player could possibly go
	printf("POSSIBLE CONNECTIONS: ");

	int i;
	//For every connection in room A
	for(i = 0; i < A.num_connections; i++){
		//If the connection is not the last one, put a comma after the name
		if(i != A.num_connections - 1){
			printf("%s, ", A.connection[i]);
		}

		//Else put a period and a newline character after the name
		else{
			printf("%s.\n", A.connection[i]);
		}
	}
	//Print a prompt to the player asking where to go
	printf("WHERE TO? >");
}

//Checks if two rooms are the same based on their name
int is_same_room(struct room A, struct room B){
	//Check if room A and room B have the same name
	if(!strcmp(A.name, B.name)){
		//If they do, return true
		return 1;
	}
	//Else return false
	return 0;
}

//A thread controlled function that writes the current system time to a file
//called currentTime.txt
void* running_time(void* input){
	//Attempt to lock the mutex. The thread will wait until it can gain access to the mutex
	//if it is already locked
	pthread_mutex_lock(&my_mutex);

	//Create an array to store the current time and initalize with '\0'
	char the_time[100];
	memset(the_time, '\0', sizeof(the_time));

	//Define variable names
	time_t mytime;
	struct tm info;

	time(&mytime);
	info = *localtime(&mytime);

	//Get the curent time in a particular format and store in "the_time" string
	strftime(the_time, sizeof(the_time), "%l:%M%P, %A, %B %d, %Y", &info);

	//Open and create a file called currentTime.txt
	int file_descriptor;
	file_descriptor = open("currentTime.txt", O_RDWR | O_CREAT, 0777);

	//Write the time string from "the_time" and store in the opened file
	ssize_t nwritten;
	nwritten = write(file_descriptor, the_time, strlen(the_time)*sizeof(char));

	//Close the file
	close(file_descriptor);	

	//Unlock the mutex so that other threads can access
	pthread_mutex_unlock(&my_mutex);
	return NULL;
}

//Retrieves the time from the file currentTime.txt and prints to the user
void get_time(){
	//Unlock the mutex and wait a second so that another thread can write the time to
	//the file currentTime.txt
	pthread_mutex_unlock(&my_mutex);
	sleep(1);

	//Create a read buffer string to take input and initalize with '\0'
	char read_buffer[1000];
	memset(read_buffer, '\0', sizeof(read_buffer));

	//Open the file currentTime.txt
	ssize_t nread;
	int file_descriptor;
	file_descriptor = open("currentTime.txt", O_RDWR | O_CREAT, 0777);

	//Start at the beginning of the file and read the whole contents into read_buffer
	lseek(file_descriptor, 0, SEEK_SET);
	nread = read(file_descriptor, read_buffer, sizeof(read_buffer));
	
	//Print the time to the user
	printf("\n%s\n", read_buffer);
	
	//Close the file
	close(file_descriptor);	
	
	//Attempt to gain back lock control from other threads
	pthread_mutex_lock(&my_mutex);

	//Create another thread that will get the current time when prompted
	int result_int, index;
	pthread_t my_thread_ID;
	result_int = pthread_create(&my_thread_ID, NULL, running_time, NULL);

}

//Atempts to return a new room location name if possible. Returns the current room
//if not. Takes the room array and the name of the current room
struct room get_new_location(struct room* my_rooms, struct room current_room){
	//Define variables
	char* buffer = NULL;         //Buffer to contain user input
	size_t buffer_size = 0;      //The size of the buffer
	int num_chars_entered = -5;  //The number of characters entered by the user

	//Create a new room object and set to NULL
	struct room new_room;
	char* new_room_name = NULL;

	//Get input from the user on possible new room locations and remove the end of
	//line character
	num_chars_entered = getline(&buffer, &buffer_size, stdin);
	buffer[num_chars_entered - 1] = '\0';

	int i, j;
	//For all rooms in the room array
	for(i = 0; i < current_room.num_connections; i++){
		//If any room in the array has the same name as the user input
		if(!strcmp(buffer, current_room.connection[i])){
			//Set the new_room_name to that name from the array
			new_room_name = current_room.connection[i];
		}
	}

	//If a new room name is found
	if(new_room_name != NULL){
		//Check all rooms in the array
		for(i = 0; i < 7; i++){
			//If that room matches the name of a room in the array
			if(!strcmp(new_room_name, my_rooms[i].name)){
				//Return that room
				new_room = my_rooms[i];
				return new_room;
			}
		}
	}

	//If the above doesnt match, check if the user entered "time"
	else if(!strcmp(buffer, "time")){
		//If so, start the function to retrieve the time and return the current room
		get_time();
		return current_room;
	}
	//If everything fails, return an error the user and return the current room
	else{
		printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
		return current_room;
	}
}

int main(){
	//Have main instantly take control of the global mutex
	pthread_mutex_lock(&my_mutex);

	//Create a thread that will take control once main unlocks the mutex
	int result_int, index;
	pthread_t my_thread_ID;
	result_int = pthread_create(&my_thread_ID, NULL, running_time, NULL);

	//Get an empty room array from the function create_empty_array()
	struct room* my_rooms = create_empty_array();

	//Get the newest directory name from get_newest_dir()
	char* dir_name = get_newest_dir();

	//Define varaibles to open the newest directory
	DIR* dir_to_check = opendir(dir_name);
	int i = 0;
	int j = 0;
	int file_descriptor;
	struct dirent *file_in_dir;
	ssize_t nread;
	char read_buffer[1000];
	char file_path[1000];
	char* token;

	//Iterate through all files in the newest directory
	while((file_in_dir = readdir(dir_to_check)) != NULL){
		//Reset the strtok token to NULL
		token = NULL;

		//If the directory is not the current or previous directory
		if(strcmp(file_in_dir->d_name, ".") != 0 && strcmp(file_in_dir->d_name, "..") != 0){

			//Reset file_path string with '\0' and then create a file path using the newest directory name
			//and the current file in that directory
			memset(file_path, '\0', sizeof(file_path));
			sprintf(file_path, "%s/%s", dir_name, file_in_dir->d_name);

			//Open the current file in read only
			file_descriptor = open(file_path, O_RDONLY);

			//Check if the file has been opened successfully and return an error if not
			if(file_descriptor == -1){
				printf("Hull breach - open() failed on \"%s\"\n", file_path);
			}

			//Clear out the read_buffer with '\0'
			memset(read_buffer, '\0', sizeof(read_buffer));
			//Start from the beginning of the file
			lseek(file_descriptor, 0, SEEK_SET);
			//Read all of the contents into read_buffer
			nread = read(file_descriptor, read_buffer, sizeof(read_buffer));

			//Use strtok to get the name of the current room from the current file
			token = strtok(read_buffer, " \n");
			token = strtok(NULL, " \n");
			token = strtok(NULL, " \n");

			//Set the room name from the strtok token to the approriate room array
			my_rooms[i].name = strdup(token);

			//Get Connection names using strtok. Do this while strtok does not encounter the room types
			do{
				token = strtok(NULL, " \n");
				token = strtok(NULL, " \n");
				token = strtok(NULL, " \n");
				
				//If strtok has not reached the room types yet
				if(strcmp(token, "MID_ROOM") && strcmp(token, "START_ROOM") && strcmp(token, "END_ROOM")){
					//Store the connection name in the appropriate place in the room array
					strcpy(my_rooms[i].connection[j], strdup(token));
				}
				//Increment the connection index
				j++;
			}while(strcmp(token, "MID_ROOM") && strcmp(token, "START_ROOM") && strcmp(token, "END_ROOM"));

			//Set the number of connections in the room array and then reset the connection index
			my_rooms[i].num_connections = j - 1;
			j = 0;

			//Get the room type using the last location of strtok and store in the room array
			my_rooms[i].type = strdup(token);

			//Increment the room array index
			i++;

			//Close the current file
			close(file_descriptor);
		}
	}
	//Close the newest directory
	closedir(dir_to_check);

	//***PLAY THE GAME***//
	
	//Define variables to play the game
	struct room current_room;   //Stores the current room
	struct room new_room;       //Stores the potential new room
	int num_steps = 0;          //Tracks the number of steps the player has taken
	char* room_path = NULL;     //Stores the room path string the player has taken, separating
	                               //room names by new line characters

	//For all rooms in the room array
	for(i = 0; i < 7; i++){
		//If the room has type START_ROOM
		if(!strcmp(my_rooms[i].type, "START_ROOM")){
			//Start the player in the START_ROOM
			current_room = my_rooms[i];
		}
	}

	//While the player has not reached the END_ROOM
	while(strcmp(current_room.type, "END_ROOM")){
		//Print the current location to the player
		print_location(current_room);	

		//Attempt to get to a new room
		new_room = get_new_location(my_rooms, current_room);

		//If the new room is not the same as the current room
		if(!is_same_room(new_room, current_room)){
			//Increment the number of steps the player has taken
			num_steps++;
			//If it's the first step
			if(num_steps == 1){
				//Create a copy of the new room name and store in room_path
				//with a new line character
				room_path = strdup(new_room.name);
				strcat(room_path, "\n");
			}
			else{
				//If it's not the first step, add the new room onto the existing room_path string
				//and add a new line character
				strcat(room_path, new_room.name);
				strcat(room_path, "\n");
			}
			//The current room gets whatever is stored in the new room variable
			current_room = new_room;
		}
		//Add a newline for readability
		printf("\n");
	}

	//Print to the player that they have won
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");

	//Tell the player how many steps they took, and what the path was to victory
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n%s", num_steps, room_path);

	//Destroy the global mutex
	pthread_mutex_destroy(&my_mutex);

	//Free allocated memory
	free(my_rooms);
	free(dir_name);
	return 0;
}
