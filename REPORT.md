# ECS 150: Project #1 - Simple Shell

## Ranxin

## Joakim

I implemented the parser such that it tokenizes the commandline arguments into 
a struct that holds an array of structs with the programs and their arguments.
parse_line() has two helper functions.

The Stack and node structs with functions are for the directory stack. The 
stack allocates memory on the heap and frees memory appropriately when nodes 
are removed. The stack is used by the built in cd program. 

Resources I've used in this project include gnu.org for documentation and I 
found this [resource](https://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/e/execvp.html) 
helpful to really understand the different versions of exec, including the difference between 'v' and 'l' and the difference between 'p' and no 'p'. This time around I started writing the parser without looking at which format exec wanted the arguments passed in, which made me have to go back and change the parsing function. 

It had been some time since I had programmed in C, so it was quite fun to work with pointers and strict data types again.

One of the tools I used for debugging was to include two commands in my Makefile, one to recompile to program, and one to recompile and run the sshell directly. 