#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//The room struct container to keep track of room parameters
struct room{
	int num_connections;  //The number of connections to other rooms
	char* name;				 //The name of the room
	char* type;           //The type of the room (END_ROOM/MID_ROOM/START_ROOM)

	//Connection 2D array with 6 as the maximum number of connections 
	//and 8 as the max length of a string (Plus the null terminator)
	char connection[6][9];
};

//Returns a random room given an array of all possible rooms
struct room get_random_room(struct room* init_array){
	//Get a random number between 0 and 9
	int random_room_int = rand() % 10;

	//Return the random room
	return init_array[random_room_int];
}

//Creates the <ONID>.buildrooms.<PID> directory and all of the rooms files within
//based on a given 7 room array
void create_room_directory(struct room* my_rooms){

	//Create a string to contain the process id, clear out the string of junk,
	//And then convert the pID int to a string
	char* process_id_string = malloc(sizeof(char)*6);
	memset(process_id_string, '\0', sizeof(process_id_string));
	sprintf(process_id_string, "%d", getpid());	

	//Create a string to contain filenames, and then blank the contents with null terminators
	char* filename = malloc(sizeof(char)*50);
	memset(filename, '\0', sizeof(filename));
	
	//Store the first part of the directory name into filename, and then concatenate the process ID
	//at the end.
	strcpy(filename, "fordc.rooms.");
	strcat(filename, process_id_string);

	//Create the directory using the above filename
	mkdir(filename, 0777);

	//Initlize variables i, j for looping, and room_name/my_str to store string data
	//Clear out the strings of junk with null terminators
	int i, j;
	char* room_name = malloc(sizeof(char)*8);
	memset(room_name, '\0', sizeof(room_name));
	char* my_str = malloc(sizeof(char)*50);
	memset(my_str, '\0', sizeof(my_str));

	//Initalize file descriptor variable before loop
	int file_descriptor;

	//Loop through all 7 rooms
	for(i = 0; i < 7; i++){
		//If it's not the first time through, reset all strings inside the loop
		//to null terminators
		if(i != 0){
						memset(room_name, '\0', sizeof(room_name));
						memset(filename, '\0', sizeof(filename));
						memset(my_str, '\0', sizeof(my_str));
		}

		//Get the name of the room and store in room_name
		strcpy(room_name, my_rooms[i].name);

		//Add the "_room" tag onto the end of the room name
		strcat(room_name, "_room");

		//Create the directory name, and then add a "/" to file path into the directory
		strcpy(filename, "fordc.rooms.");
		strcat(filename, process_id_string);
		strcat(filename, "/");

		//Add the room name with pID onto the end of the filename to create the file path
		//to the room file
		strcat(filename, room_name);

		//Initalize read and write variables for file creation
		ssize_t nread, nwritten;

		//Create and open a room file for a given room in the array
		file_descriptor = open(filename, O_RDWR | O_CREAT | O_APPEND, 0777);

		//Check if the creation was successful
		if(file_descriptor == -1){
			printf("Could not open file %s\n", filename);
			exit(1);
		}

		//Create the room name line with the given room name at room array location i and store in my_str
		//Reset my_str with null terminators after use
		sprintf(my_str, "ROOM NAME: %s\n", my_rooms[i].name);
		nwritten = write(file_descriptor, my_str, strlen(my_str)*sizeof(char));
		memset(my_str, '\0', sizeof(my_str));
		
		//Create all of the connection names at room array location i,j and store in my_str
		//Reset my_str with null terminators after use
		for(j = 0; j < my_rooms[i].num_connections; j++){
			sprintf(my_str, "CONNECTION %d: %s\n", j+1, my_rooms[i].connection[j]);
			nwritten = write(file_descriptor, my_str, strlen(my_str)*sizeof(char));
			memset(my_str, '\0', sizeof(my_str));
		}

		//Create the room type line with the given type at room array location i and store in my_str
		//Reset my_str with null terminantors after use
		sprintf(my_str, "ROOM TYPE: %s\n", my_rooms[i].type);
		nwritten = write(file_descriptor, my_str, strlen(my_str)*sizeof(char));
		memset(my_str, '\0', sizeof(my_str));

		//Close the created room file for the given room in the array
		close(file_descriptor);
	}

	//Clear all allocated memory used in this function
	free(room_name);
	free(my_str);
	free(filename);
	free(process_id_string);
}

//Returns the initalized room array that contains all possible rooms
struct room* define_rooms(){

	//Allocate space for 10 rooms in an array
	struct room* init_array = malloc(sizeof(struct room)*10);

	//Hard code the names of all possible rooms
	init_array[0].name = "cellar";
	init_array[1].name = "church";
	init_array[2].name = "yard";
	init_array[3].name = "kitchen";
	init_array[4].name = "ballroom";
	init_array[5].name = "lobby";
	init_array[6].name = "bedroom";
	init_array[7].name = "bathroom";
	init_array[8].name = "barracks";
	init_array[9].name = "sanctum";

	//For all rooms in the array, set the number of connections to zero for default
	int i;
	for(i = 0; i < 10; i++){
		init_array[i].num_connections = 0;
	}

	//Return the initalized array
	return init_array;
}


//Recieves the initalized room array of all possible rooms, and returns a random set of 7 rooms
//in an array with no duplicates
struct room* initalize_graph(struct room* init_array){
	//Allocate space for 7 rooms in an array
	struct room* my_rooms = malloc(sizeof(struct room)*7);

	//Loop through for each room in the new array
	int i;
	for(i = 0; i < 7; i++){
		//Call the get random room function using the initial array as an arguement
		my_rooms[i] = get_random_room(init_array);	
		int j;
		//Loop through the new array
		for(j = 0; j < 7; j++){
			//Look through all rooms currently in the new array
			//If there's a duplicate, decriment i and restart the search
			//at that index
			if(j < i){
				if(!strcmp(my_rooms[i].name, my_rooms[j].name)){
					i--;
					break;
				}
			}
		}
	}

	//Return the new array of 7 random and unique rooms
	return my_rooms;
}

//Frees a given room array
void free_room_array(struct room* room_memory){
	free(room_memory);
}

//Checks if the graph is full. Full is defined by having all rooms have at least 3 connections
//A given room array is used as an arugement
int is_graph_full(struct room* my_rooms){
	//Loop through all rooms in the array
	int i;
	for(i = 0; i < 7; i++){
		//If the number of connections is less than 3, return false
		if(my_rooms[i].num_connections < 3){
			return 0;
		}
	}
	//Return true if the check passed
	return 1;
}

//Connects a given room A and B using room pointers to ensure
//pass by reference
void connect_rooms(struct room* A, struct room* B){
	//Add a connection from room A to Room B by name, and then increment the number of connections
	strcpy(A->connection[A->num_connections], B->name);
	A->num_connections += 1;

	//Add a connection from room B to Room A by name, and then increment the number of connections
	strcpy(B->connection[B->num_connections], A->name);
	B->num_connections += 1;
}

//Checks if a room has reached or exceeded the maximum number of connections
int can_add_connection(struct room A){
	//For a given room A, if the number of connections is 6 or more, return false
	if(A.num_connections >= 6){
		return 0;
	}
	//If the test passes, return true
	return 1;
}

//Checks if a connection already exists for given rooms A and B.
int connection_exists(struct room A, struct room B){
	//Check all possible connections from 0-6
	int i;
	for(i = 0; i < 6; i++){
		//If the connection names are the same at index i, return true
		if(!strcmp(A.connection[i], B.name) || !strcmp(B.connection[i], A.name)){
			return 1;
		}
	}
	//If the check does not trigger, return false
	return 0;
}

//Checks if given rooms A and B are the same room
int is_same_room(struct room A, struct room B){
	//If the names of room A and B are the same, return true
	if(!strcmp(A.name, B.name)){
		return 1;
	}

	//If the check does not trigger, return false
	return 0;
}

//Adds a random connection between rooms in a given room array
void add_random_connection(struct room* my_rooms){
	//Initalize variables Aptr and Bptr to allow for rooms to pass by reference, and rand_num
	//to contain a random number
	struct room* Aptr;
	struct room* Bptr;
	int rand_num;

	//Continue until a room is chosen
	while(1){
		//Generate a random number between 0-6, and then point Aptr to that room
		rand_num = rand() % 7;
		Aptr = &my_rooms[rand_num];

		//If the room has less than 6 connections, then break out of while
		if(can_add_connection(*Aptr) == 1){
			break;
		}
	}

	//Continually select rooms for Bptr until the room chosen is not the same as the room addressed by Aptr,
	//A connection does not already exist, and the room has less than 6 connections
	do{
		//Generate a random number between 0-6, and then point Bptr to that room
		rand_num = rand() % 7;
		Bptr = &my_rooms[rand_num];
	}while(can_add_connection(*Bptr) == 0 || is_same_room(*Aptr, *Bptr) == 1 || connection_exists(*Aptr, *Bptr) == 1);

	//Connect the given rooms
	connect_rooms(Aptr, Bptr);
}

//Adds a type to all rooms (END_ROOM, MID_ROOM, START_ROOM) in a given room array
//There is only one END and START room
void add_room_type(struct room* my_rooms){
	//Randomly assign the end room to the array of rooms
	int rand_num = rand() % 7;
	my_rooms[rand_num].type = "END_ROOM";

	//Randomly pick a start room, as long is it's not already the end room
	while(my_rooms[rand_num].type == "END_ROOM"){
		rand_num = rand() % 7;
	}
	my_rooms[rand_num].type = "START_ROOM";
	
	//Iterate through all rooms in the array, and assign them the mid room as long
	//as the room is not already the start or end room
	int i;
	for(i = 0; i < 7; i++){
		if(my_rooms[i].type != "END_ROOM" && my_rooms[i].type != "START_ROOM"){
			my_rooms[i].type = "MID_ROOM";
		}
	}

}

int main(){
	//Seed the random number generator
	srand(time(NULL));

	//Create the array of all possible rooms
	struct room* init_array = define_rooms();

	//From the inital array of all possible rooms, generate a random array of 7 rooms
	//with no duplicates
	struct room* my_rooms = initalize_graph(init_array);

	//Create random connections between rooms in the newly created array until it is full
	while(is_graph_full(my_rooms) == 0){
		add_random_connection(my_rooms);
	}

	//Give all rooms in the create room array a type
	add_room_type(my_rooms);

	//Create a dirctory that contains files for all newly created rooms
	create_room_directory(my_rooms);

	//Free all memory used to contain rooms in an array
	free_room_array(init_array);
	free_room_array(my_rooms);

	return 0;
}


