fakemake: hmap.o
	gcc main.c hmap.o -o fakemake
hmap.o: hmap.c
	gcc -c hmap.c
clean:
	rm fakemake hmap.o
