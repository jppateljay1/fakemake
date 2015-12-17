/*
 * Jay Patel
 * file://hmap.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmap.h"

int SYSTEM_TIME;
unsigned Coeff1 = 27;

int Pwrs_2[] = {
	16, 32, 64, 128, 256,
	512, 1024, 2048, 4096, 8192,
	16384, 32768, 65536, 131072, 262144,
	524288, 1048576, 2097152, 4194304, 8388608,
	16777216, 33554432, 67108864, 134217728, 268435456,
	536870912, 1073741824, 2147483648
};

int Primes[] = {
	53, 97, 193, 389, 769,
	1543, 3079, 6151, 12289, 24593,
	49157, 98317, 196613, 393241, 786433,
	1572869, 3145739, 6291469, 12582917, 25165843,
	50331653, 100663319, 201326611, 402653189, 805306457,
	1610612741};

int Primes_P1[] = {
	53+1, 97+1, 193+1, 389+1, 769+1,
	1543+1, 3079+1, 6151+1, 12289+1, 24593+1,
	49157+1, 98317+1, 196613+1, 393241+1, 786433+1,
	1572869+1, 3145739+1, 6291469+1, 12582917+1, 25165843+1,
	50331653+1, 100663319+1, 201326611+1, 402653189+1, 805306457+1,
	1610612741+1};



int *SizeTables[] = {Pwrs_2, Primes, Primes_P1};

char *SizePolicies[] = {"POWERS-OF-TWO", "PRIMES", "PRIMES_PLUS_1"};


/******** STRUCTS AND TYPEDEFS *********/
typedef struct node_struct
{
    char *key;
    void *val;
    unsigned hval;
    int time_stamp;
    int dependency;
    struct node_struct *next;
}NODE;

typedef struct {
    NODE *members;
    int n;
} TBL_ENTRY;

struct hmap {
    TBL_ENTRY *tbl;
    int *tsize_tbl;
    char *tsize_policy;
    int tsize_idx;
    int tsize;
    int n; 
    double lfactor;
    int max_n;
    HFUNC hfunc;
    char *hfunc_desc;
};

typedef struct {
	HFUNC hfunc;
	char *description;
} HFUNC_STRUCT;

/******** END STRUCTS AND TYPEDEFS *********/

/******** LIBRARY SUPPLIED HASH FUNCTIONS *****/

static unsigned h0(char *s) {
unsigned h = 0;

  while(*s != '\0'){
	h += *s;
	s++;
  }
  return h;
}


static unsigned int h1(char *s) {
unsigned h=0;

  while(*s != '\0') {
	h = h*Coeff1 + *s;
	s++;
  }
  return h;
}
/******** END LIBRARY SUPPLIED HASH FUNCTIONS *****/



/***** GLOBALS TO THIS FILE ******/

static HFUNC_STRUCT HashFunctions[] = 
	{ 
		{h0, "naive char sum"},
		{h1, "weighted char sum"}
	};


static int NumHFuncs = sizeof(HashFunctions)/sizeof(HFUNC_STRUCT);

/***** END GLOBALS ******/

/***** FORWARD DECLARATIONS *****/
static int match(char *key, unsigned hval, NODE *p); 
static NODE **get_node_pred(HMAP_PTR map, char *key); 
static TBL_ENTRY *create_tbl_array(int tsize); 
static void resize(HMAP_PTR map); 
static void free_lst(NODE *l, int free_vals); 
//static void free_lst(NODE *l); 
static void add_front(TBL_ENTRY *entry, NODE *p);
static int get_key_helper(HMAP_PTR map, char *filename);
static void add_back(TBL_ENTRY *entry, NODE *p);
static NODE *get_NODE(HMAP_PTR map, int token);
/***** END FORWARD DECLARATIONS *****/


/***** BEGIN hmap FUNCTIONS ******/

int is_null(HMAP_PTR map, int token)
{
  if(map->tbl[token].members == NULL)
    return 0;

  return 1;
}

char *return_key(HMAP_PTR map, int token)
{
  return (map->tbl[token].members->key);
}

int return_dependency(HMAP_PTR map, int token)
{
  return (map->tbl[token].members->dependency);
}

int get_map_size(HMAP_PTR map)
{
  return map->tsize;
}

/*
 * primiarly used with to check if there are any cycles present 
 * in the makefile
 */
int user_cycle(HMAP_PTR map, char *word)
{
  if(hmap_contains(global_repeat, word))
  {
    printf("------------------------------\n");
    printf("cycle detected\n");
    printf("------------------------------\n");
    return 0;
  }

  if(!hmap_contains(map, word))
  {
    printf("-------------------------------------------------\n");
    printf("ERROR: %s does not exist\n", word);
    printf("-------------------------------------------------\n");
    return 0;
  }

  //adds the file on the hmap
  hmap_set(global_repeat, word, NULL, BASIC);

  int update_status = 0;
  int token = get_key(map, word);
  NODE *p = map->tbl[token].members;
  int timep = p->time_stamp;

  NODE *pnext;

  for(pnext = p->next; pnext != NULL; pnext = pnext->next)
  {
    if(pnext->dependency == BASIC)
    {
      if(timep < user_timestamp(map, pnext->key))
        update_status = 1;
    }
    else
    {
      if(!user_cycle(map, pnext->key))
        return 0;
    }
  }

  //at this point, no cycle was detected
  return 1;
}

int user_make(HMAP_PTR map, char *word)
{	
  int update_status = 0;
  int token = get_key(map, word);

  NODE *p = map->tbl[token].members;
  if(p->dependency == BASIC)
  {
    printf("ERROR: %s is a basic file, not dependent\n", word);
    return -999;
  }

  int timep = p->time_stamp;

  NODE *pnext;

  for(pnext = p->next; pnext != NULL; pnext = pnext->next)
  {
    if(pnext->dependency == BASIC)
    {
      if(timep < user_timestamp(map, pnext->key))
        update_status = 1;
    }
    else
    {
      if(user_make(map, pnext->key) == 1)
        update_status = 1;
      else
      {
        if(timep < user_timestamp(map, pnext->key))
        update_status = 1;
      }
    }
  }

  //if the status is updated, then we have to update the time too
  if(update_status == 1)
  {
    p->time_stamp = SYSTEM_TIME;
    SYSTEM_TIME++;
    map->tbl[token].members = p;
    printf("making....%s\n", p->key);
    return 1;
  }

  //at this point, nothing had to be changed
  printf("%s is up to date\n",p->key);
  return 0;
}

int user_timestamp(HMAP_PTR map, char *word)
{
  //before anything happens, checks to see if the word actually exist
  if(!hmap_contains(map, word))
    return -999;

  return (map->tbl[get_key(map, word)].members->time_stamp);
}

void touch_user(HMAP_PTR map, char *word)
{
  //before anything happens, checks to see if the word actually exist
  if(!hmap_contains(map, word))
  {
    printf("ERROR: %s, does not exist\n", word);
    return;
  }

  int token = get_key(map, word);
  NODE *p = map->tbl[token].members;

  if(p->dependency == DEPENDENCY)
  {
    printf("ERROR: trying to change time on a dependency file\n");
    return;
  }
  else
  {
    p->time_stamp = SYSTEM_TIME;
    SYSTEM_TIME++;
  }

  map->tbl[token].members = p;
}

void add_next_word(HMAP_PTR map, char *word, char *next_word, int dep)
{
  int token = get_key(map, word);

  NODE *p = map->tbl[token].members;

  if(p->next == NULL)
  {
    NODE *pnext = malloc(sizeof(NODE));
    char *key_clone;
    key_clone = malloc( (strlen(next_word) + 1)*sizeof(char));
    strcpy(key_clone, next_word);
    pnext->key = key_clone;
    pnext->dependency = dep;
    pnext->next = NULL;
    p->next = pnext;
  }
  else //p->next != NULL and we can just push the new node next of p
  {
    NODE *pnext = malloc(sizeof(NODE));
    char *key_clone;
    key_clone = malloc( (strlen(next_word) + 1)*sizeof(char));
    strcpy(key_clone, next_word);
    pnext->key = key_clone;
    pnext->dependency = dep;
    pnext->next = p->next;
    p->next = pnext;  
  }

  map->tbl[token].members = p;
}

/*
 * prints all the time stamps for the file
 */
void print_all_time(HMAP_PTR map)
{
  int size = map->tsize, i;
  for(i = 0; i < size; i++)
  {
    if(map->tbl[i].members == NULL)
      continue;
    printf("%s time: %d\n", map->tbl[i].members->key, map->tbl[i].members->time_stamp);
  }
}

void read_from_user(HMAP_PTR map)
{
  SYSTEM_TIME = 1;
  char input[30];
  
  while(1)
  {
    printf("---------------------------------------------------\n");
    printf("Enter a command or ? for list of commands\n");

    gets(input);

    if(strcmp(input, "quit") == 0)
      break;
    else if(strcmp(input, "?") == 0)
    {
      printf("time\n");
      printf("timestamps\n");
      printf("touch <filename>\n");
      printf("timestamp <filename>\n");
      printf("make <filename>\n");
    }
    else if(strcmp(input, "time") == 0)
    {
      //print the current time of the clock
      printf("current time clock: %d\n", SYSTEM_TIME);
    }
    else if(strcmp(input, "timestamps") == 0)
    {
      //prints time stamps for all the files
      print_all_time(map);
    }
    else
    {
      char temp[20];
      char *file_name;
      if(input[0] == 't' && input[1] == 'o')//touch
      {
        sscanf(input, "%s %s", temp, file_name);
        touch_user(map, file_name);
      }
      else if(input[0] == 't' && input[1] == 'i') //timestamp
      {
        sscanf(input, "%s %s", temp, file_name);

        int ret_time = user_timestamp(map, file_name);
        if(ret_time == -999)
          printf("ERROR: %s, does not exist\n", file_name);
        else
          printf("%s was last updated at time %d\n", file_name, ret_time);
      }
      else if(input[0] == 'm') //make
      {
        sscanf(input, "%s %s", temp, file_name);
        user_make(map, file_name);
      }
      else
      {
        printf("Invalid entry, enter again\n");
      }
    }

    printf("---------------------------------------------------\n");
  }
}

int map_everything(HMAP_PTR map, FILE *fin)
{
    char line[1000];
    // read a line from a file 
    while(fgets(line,sizeof line,fin)!= NULL)
    {
      fprintf(stdout,"%s",line); //print the file contents on stdout.

      //gets rid of all the \n values to make code readable
      int i;
      for(i = 0; i < 1000; i++)
      {
        if(strcmp(&line[i], "\n") == 0)
        {
          line[i] = ' ';
          break;
        }
      }

        char *word = strtok(line, " ");
        char *word_two = strtok(NULL, " ");
        if(word_two == NULL)
        {
          //checks if the word already exists  
          if(hmap_contains(map, word))
          {
              printf("ERROR: word exists, %s\n", word);
              return 0;
          }
          hmap_set(map, word, NULL, BASIC);
        }
        else if(strcmp(word_two, ":") == 0)
        {
            if(hmap_contains(map, word))
            {
                printf("ERROR: word exists, %s\n", word);
                return 0;
            }
            hmap_set(map, word, NULL, DEPENDENCY);

            char *temp = strtok(NULL, " ");
            while(temp != NULL)
            {
                /*
                 * checks to see if this is a dependency or a basic file
                 * 0 = dependency
                 * 1 = basic
                 */
                if(hmap_contains(map, temp) == 0) 
                {
                  //detects if we are trying to add any null space at the end
                  if(strcmp(temp, "\n") == 0)
                  {
                    temp = strtok(NULL, " ");
                    continue;
                  }
                  add_next_word(map, word, temp, DEPENDENCY);
                }
                else
                {
                  int temp_token = get_key(map, temp);
                  NODE *to_delete = map->tbl[temp_token].members;

                  if(to_delete->dependency == DEPENDENCY)
                    add_next_word(map, word, temp, DEPENDENCY);
                  else
                    add_next_word(map, word, temp, BASIC);
                }

                temp = strtok(NULL, " "); //gets the next word in line
            }
        }
        else
        {
          //checks if the word already exists
            if(hmap_contains(map, word))
            {
                printf("ERROR: word exists %s\n", word);
                return 0;
            }
            hmap_set(map, word, NULL, BASIC);
        }
    }
    //at this point, everything ran succesfully
    return 1;
}

static int get_key_helper(HMAP_PTR map, char *filename)
{
  unsigned h = map->hfunc(filename);
  int indx = h % map->tsize;

  return indx;
}

int get_key(HMAP_PTR map, char *filename)
{
  return get_key_helper(map, filename);
}

char *get_word(HMAP_PTR map, int token)
{
  return (map->tbl[token].members->key);
}

void print_all_dependency(HMAP_PTR map, char *filename)
{
  int token = get_key(map, filename);

  NODE *p = map->tbl[token].members;
  NODE *pb = p;

  while(p != NULL)
  {
    printf("name: %s\n", p->key);
    p = p->next;
  }
}


HMAP_PTR hmap_create(unsigned init_tsize, double lfactor){

  return hmap_create_size_policy(init_tsize, lfactor,
				 DEFAULT_TSIZE_POLICY);

}

HMAP_PTR hmap_create_size_policy(unsigned init_tsize, 
			double lfactor, int tsize_mode){
HMAP_PTR map = malloc(sizeof(struct hmap));
int idx;

  map->n = 0;
  if(lfactor <= 0)
	lfactor = DEFAULT_LFACTOR;
  if(init_tsize <= 0)
	init_tsize = DEFAULT_INIT_SIZE;
  
  map->lfactor = lfactor;

  map->tsize_tbl = SizeTables[tsize_mode];
  map->tsize_policy = SizePolicies[tsize_mode];
  idx = 0;
  while(map->tsize_tbl[idx] < init_tsize)
	idx++;

  map->tsize = map->tsize_tbl[idx];
  map->tsize_idx = idx;
  map->max_n = map->tsize * lfactor;

  map->hfunc = HashFunctions[DEFAULT_HFUNC_ID].hfunc;
  map->hfunc_desc = HashFunctions[DEFAULT_HFUNC_ID].description;

  map->tbl = create_tbl_array(map->tsize);

  map->n = 0;
 
  return map;
}

void hmap_set_coeff(unsigned new_coeff){

  Coeff1 = new_coeff;

}
  

int hmap_size(HMAP_PTR map) {
  return map->n;
}

void hmap_display(HMAP_PTR map) {
int i, j;
  
  for(i=0; i<map->tsize; i++) {
      printf("|-|");
      for(j=0; j<map->tbl[i].n; j++) 
	  printf("X");
      printf("\n");
  }
}
int hmap_set_hfunc(HMAP_PTR map, int hfunc_id) {
  if(map->n > 0) {
	fprintf(stderr, 
	  "warning:  attempt to change hash function on non-empty table\n");
	return 0;
  }
  if(hfunc_id < 0 || hfunc_id >= NumHFuncs) {
	fprintf(stderr, 
	  "warning:  invalid hash function id %i\n", hfunc_id);
	return 0;
  }
  map->hfunc = HashFunctions[hfunc_id].hfunc;
  map->hfunc_desc = HashFunctions[hfunc_id].description;
  return 1;
}

int hmap_set_user_hfunc(HMAP_PTR map, HFUNC hfunc, char *desc) {
  if(map->n > 0) {
	fprintf(stderr, 
	  "warning:  attempt to change hash function on non-empty table\n");
	return 0;
  }
  map->hfunc = hfunc;
  if(desc == NULL)
    map->hfunc_desc = "user-supplied hash function";
  else 
    map->hfunc_desc = desc;
  return 1;
}



int hmap_contains(HMAP_PTR map, char *key) {
NODE **pp;
  pp = get_node_pred(map, key);
  return (*pp == NULL ? 0 : 1);
}

void *hmap_get(HMAP_PTR map, char *key) {
NODE **pp;
  pp = get_node_pred(map, key);
  return (*pp == NULL ? NULL : (*pp)->val);
}

void * hmap_set(HMAP_PTR map, char *key, void *val, int dependency){
unsigned h;
int idx;
NODE *p, **pp;

  pp = get_node_pred(map, key);
  p = *pp;

  if(p == NULL) {  // key not present
     char *key_clone;

     key_clone = malloc( (strlen(key) + 1)*sizeof(char));
     strcpy(key_clone, key);

     map->n++;
     if(map->n > map->max_n) 
	resize(map);
     h = map->hfunc(key);
     idx = h % map->tsize;

     p = malloc(sizeof(NODE));

     p->key = key_clone;
     p->val = val;
     p->hval = h;
     p->time_stamp = 0;
     p->dependency = dependency;

     add_front(&(map->tbl[idx]), p);
     return NULL;  // no old value to return
  }
  else {  // key already in map
     void *tmp = p->val;
     p->val = val;
     return tmp;  // return old value
  }
}

void hmap_insert(HMAP_PTR map, char *key) {

  if(hmap_contains(map, key))
	return;
  hmap_set(map, key, NULL, 0);

}

void *hmap_remove(HMAP_PTR map, char *key) {
NODE *p, **pp;

  pp = get_node_pred(map, key);
  p = *pp;
  if(p == NULL){
	return NULL;
  }
  else {
	unsigned idx;
	void *val = p->val;

	*pp = p->next;  // make predecessor skip node
			//   being removed
	free(p->key);
	free(p);

  	idx = (p->hval) % map->tsize;
	map->tbl[idx].n--;
	map->n--;
	return val;
  }
}
  

static int max_len(HMAP_PTR map) {
int i;
int max = 0;

  for(i=0; i<map->tsize; i++) 
	if(map->tbl[i].n > max)
		max = map->tbl[i].n;
  
  return max;
}

static void histogram(HMAP_PTR map) {
int max = max_len(map);
int cutoff;
int freq[max+1];
int i, len ;

  for(i=0; i<=max; i++)
    freq[i]=0;

  for(i=0; i<map->tsize; i++) {
	len = map->tbl[i].n;
	freq[len]++;
  }
  printf("\n\n   DISTRIBUTION OF LIST LENGTHS\n\n");

  printf("NBUCKETS:   ");
  for(len=0; len <= max; len++) {
	printf("%7i",  freq[len]);
  }
  printf("\n");
  printf("------------");
  for(len=0; len <= max; len++) {
	printf("-------");
  }
  printf("\n");
  printf("LIST-LEN:   ");
  for(len=0; len <= max; len++) {
	printf("%7i",  len);
  }
  printf("\n\n");
  
}

static double avg_cmps(HMAP_PTR map) {
int i;
double total = 0;

  for(i=0; i<map->tsize; i++) {
	int ni = map->tbl[i].n;
	total += (ni*(ni+1))/2;
  }
  return total/map->n;
}

void hmap_print_stats(HMAP_PTR map) {


    printf("######## TABLE STATS ##########\n");

    printf("   hash-func:             %s \n", map->hfunc_desc);
    printf("   tsize-policy:          %s \n", map->tsize_policy);
    printf("   tblsize:               %i \n", map->tsize);
    printf("   numkeys:               %i \n", map->n);
    printf("   max-collisions:        %i \n", max_len(map));
    printf("   avg-cmps-good-lookup:  %f \n", avg_cmps(map));

    histogram(map);

    printf("###### END TABLE STATS ##########\n");

}

void ** hmap_extract_values(HMAP_PTR map) {
int i, k;
NODE *p;
void **values = malloc(map->n * (sizeof(void *)));

  k=0;
  for(i=0; i<map->tsize; i++) {
	p = map->tbl[i].members;
	while(p != NULL) {
		values[k] = p->val;
		k++;
		p = p->next;
	}
  }
  return values;
}
  
void hmap_free(HMAP_PTR map, int free_vals) 
//void hmap_free(HMAP_PTR map) 
{
int i;

  for(i=0; i<map->tsize; i++) 
	free_lst(map->tbl[i].members, free_vals);
  //free_lst(map->tbl[i].members);
  free(map->tbl);
  map->tbl = NULL;  // not needed
  free(map);
}



/**** UTILITY FUNCTIONS *******/

static int match(char *key, unsigned hval, NODE *p) {
  return (p->hval == hval && strcmp(key, p->key)==0);
}

static NODE **get_node_pred(HMAP_PTR map, char *key) {
unsigned h;
int i;
NODE **pp;
  h = map->hfunc(key);
  i = h % map->tsize;

  pp =&(map->tbl[i].members); 
  while( *pp != NULL) {
    if(match(key, h, *pp)) 
	return pp;
    pp = &((*pp)->next);
  }
  return pp;
}

static void add_front(TBL_ENTRY *entry, NODE *p) {
  entry->n++;
  p->next = entry->members;
  entry->members = p;
}

static TBL_ENTRY *create_tbl_array(int tsize) {
int i;
TBL_ENTRY *tbl;
NODE *p;

  tbl = malloc(tsize * sizeof(TBL_ENTRY));
  for(i=0; i<tsize; i++) {
	tbl[i].members = NULL;
	tbl[i].n = 0;
  }
  return tbl;
}

static void resize(HMAP_PTR map) {
int ntsize;
TBL_ENTRY *ntbl;
NODE *nxt, *p;
unsigned h;
int i, idx;

  map->tsize_idx++;
  ntsize = map->tsize_tbl[map->tsize_idx];
  ntbl = create_tbl_array(ntsize);

  for(i=0; i<map->tsize; i++) {
    p = map->tbl[i].members;
    while(p != NULL) {
	nxt = p->next;
  	// h = map->hfunc(key);  // no need to recompute
  	idx = p->hval % ntsize;
	add_front(&ntbl[idx], p);
	p = nxt;
    }
  }
  free(map->tbl);
  map->tbl = ntbl;
  map->tsize = ntsize;
  map->max_n = (int)(ntsize * map->lfactor);

}
static void free_lst(NODE *l, int free_vals) {
//static void free_lst(NODE *l) {
  if(l == NULL) return;
  free_lst(l->next, free_vals );
  //free_lst(l->next);
  free(l->key);  // made our own copy
  if(free_vals &&  l->val != NULL)
	free(l->val);
  free(l);
}
/**** END UTILITY FUNCTIONS *******/



