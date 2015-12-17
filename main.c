/*
 * Jay Patel
 * file://main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmap.h"

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("---------------------------------------------------\n");
		printf("Enter a filename after the executable\n");
		printf("ex: ./fakemake <filename>\n");
		printf("---------------------------------------------------\n");
		exit(-1);
	}

	char file_name[50];

	strcpy(file_name, argv[1]);

	FILE *fin;
	fin = fopen(file_name, "r");

	if(fin == NULL)
	{
		fprintf(stderr, "can not open input file, %s\n", file_name);
		return 0;
	}

	/*
	 * at this point, file is valid and exist in the directory
	 * now, we can start reading line by line and start adding words on to the hash map
	 */

	HMAP_PTR map = hmap_create(0, 1.0);

	/*
	 * now that we have our hash map created
	 * we can start mapping words to it
	 */

    if(!map_everything(map, fin))
        goto END;

    // checks to see if there is any cycle present in the code
    int i;
    int size = get_map_size(map);
    for(i = 0; i < size; i++)
    {
    	if(!is_null(map, i))
    		continue;
    	if(return_dependency(map, i))
    	{
    		global_repeat = hmap_create(0, 1.0);
    	//	char *new_key = return_key(map, i);
    		if(!user_cycle(map, return_key(map,i)))
    			goto END;
    		hmap_free(global_repeat,1);
    	}
    }

    /*
     * at this point, the program has not detected any cycles
     * 
     * the program goes into an interactive mode
     * the user is able to use the make commands
     */
	read_from_user(map);

END:
    //closes the file
    fclose(fin);

	//frees the hash table
	hmap_free(map,1);
	printf("end\n");

	return 0;
}















