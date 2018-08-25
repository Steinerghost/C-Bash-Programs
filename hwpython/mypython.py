import random
import sys

#Initialize a random string variable
random_string = ""

#For each random string file
for i in range(0,3):
	#Define the filename prefix
	filename = "string_"

	#Empty the random_string
	random_string = ""

	#For each letter in a given random string file
	for j in range(0,10):
		#Generate a random ascii int from 97 to 122 to cover a-z
		random_lower_char_ascii = random.randint(97,122)

		#Convert the ascii to a char and then append to the random_string
		random_string = random_string + str(unichr(random_lower_char_ascii))

	#Add a newline character at the end of the random string
	random_string = random_string + '\n'

	#Create a unique filename using the i index. End with the txt file extention
	filename = filename + str(i+1) + ".txt"

	#Open the file with the given filename
	f = open(filename, "w+")

	#Write the string to the opened file
	f.write(random_string)

	#Close the file
	f.close()

	#Write the file to stdout
	sys.stdout.write(random_string)

#Create two random integers between 1 and 42
rand_int_one = random.randint(1,42)
rand_int_two = random.randint(1,42)

#Print both random integers
sys.stdout.write(str(rand_int_one) + '\n')
sys.stdout.write(str(rand_int_two) + '\n')

#Print the product of the two integers
sys.stdout.write(str(rand_int_one * rand_int_two) + '\n')
