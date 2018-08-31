#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char* argv[]){
	srand(time(NULL));

	int i;
	if(argc == 1){
		fprintf(stderr, "Error: Must provide size and optional redirect.\n");
		exit(1);
	}

	if(argc > 1){
		for(i = 0; argv[1][i] != 0; i++){
			if(!isdigit(argv[1][i])){
				fprintf(stderr, "Error: First arguement must be a positive integer.\n");
				exit(1);
			}
		}
	}
	int string_size = atoi(argv[1]);

	char random_string[string_size + 2];
	memset(random_string, '\0', sizeof(random_string));

	char rand_letter;
	int space_chance;
	for(i = 0; i < string_size; i++){
		space_chance = rand() % (27 + 1 - 1) + 1;
		if(space_chance == 1){
			random_string[i] = 32;   //" "
		}
		else{
			rand_letter = rand() % (90 + 1 - 65) + 65;  //A-Z
			random_string[i] = rand_letter;
		}
	}
	random_string[i] = '\n';
	printf("%s", random_string);

	return 0;
}
