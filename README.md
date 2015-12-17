# fakemake
The program mimics the behavior of the UNIX make program
/*
 * Jay Patel
 * file://readme.txt
 */

Type "make fakemake" to compile
./fakemake <filename> to run the program
type "?" to see available commands and follow on screen procedures

To detect any repeated cycle, you have to actually use the make <filename> command

Other sanity checkers have been added to eliminate any errors within the program. For example, if you try to make a basic file, it will print out an error. Also, if you have a basic file with in the dependency, it will also check that as well.
For example: let's say we have this makefile for example
	hmap.c
	hmap.h
	util.c
	util.o : util.c util.h
	hmap.o : hmap.c hmap.h

This will result in an error, because "util.h" does not actually exist.

It will also detect if you are trying to add a dependency that has already been used.
For example: let's say we have this makefile for example
	hmap.c
	hmap.h
	util.c
	util.o : util.c util.h
	util.o : hmap.c hmap.h

This will result in an error as well since "util.o" is being used twice.

It will also detect an error when you are trying to make a file that does not exist. So, from the previous example, if you try make util.c, it will result in an error saying that you are trying to make a basic file, which is not allowed.

Also, from the above example, if you try to make sum.o, it will result in an error, since "sum.o" does not even exist.

