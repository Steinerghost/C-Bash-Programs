/************************************************************************
 * Author: Casey Ford
 * Date: 6/10/18
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char* argv[]){
	//Seed the srand function with the time
	srand(time(NULL));

	//Check the number of requirements
	//If the number is only one, print error on usage and exit
	int i;
	if(argc == 1){
		fprintf(stderr, "Error: Must provide size and optional redirect.\n");
		exit(1);
	}

	//If there are more than one arguement, check the second arguement to ensure
	//it's a positive integer
	if(argc > 1){
		for(i = 0; argv[1][i] != 0; i++){
			if(!isdigit(argv[1][i])){
				fprintf(stderr, "Error: First arguement must be a positive integer.\n");
				exit(1);
			}
		}
	}
	//Store the string size in a variable
	int string_size = atoi(argv[1]);

	//The random string will be size of string_size + 2 to contain newline and '\0'
	char random_string[string_size + 2];
	memset(random_string, '\0', sizeof(random_string));

	//Initalize variables for random string generation
	char rand_letter;
	int space_chance;

	//For each index in the random string
	for(i = 0; i < string_size; i++){
		//Create a random number between 1-27
		space_chance = rand() % (27 + 1 - 1) + 1;
		//If the number is 1, the location is a space
		if(space_chance == 1){
			random_string[i] = 32;   //32 is the ASCII for " "
		}
		//If the number isn't 1, then the location is a capital letter
		else{
			//Generate a random ASCII value between 65-90
			rand_letter = rand() % (90 + 1 - 65) + 65;  //A-Z
			//Insert that ASCII into the index location
			random_string[i] = rand_letter;
		}
	}
	//End the random string with a newline character
	random_string[i] = '\n';

	//Print the random_string to stdout
	printf("%s", random_string);

	return 0;
}
