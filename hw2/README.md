This is a text adventure using two c files.

The first file is fordc.buildrooms.c which creates a directory containing 7 rooms names.
Each rooms has 3-6 connections to another room. One room is the start room while another
is the end room.

Use the following to run fordc.buildrooms.c
    compile:
        gcc -o fordc.buildrooms fordc.buildrooms.c
    run:
        ./fordc.buildrooms

The second file is fordc.adventure.c which runs the game based on the most
recent buildrooms directory. You start in the start room, and must navigate to the end room.
The command "time" can be used to get the current time. This is implemented using threading and mutexs.

Use the following to run fordc.adventure.c
    compile:
        gcc -o fordc.adventure fordc.adventure.c
    run:
        ./fordc.adventure

