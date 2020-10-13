#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* CONSTANTS */
#define MAX_CYCLES 1000000 /* maximum amount of memory cycles */
#define MAX_MODULES 2048 /* maximum amount of memory modules for the simulation */

/* STRUCTS */
struct memory_module {
  int free;
};

struct processor {
  int request, access_counter, priority;
  double cumulative_average, granted;
};

/* FUNCTION DECLARATIONS */
void S(int, int, char*);
void uniform_distribution(int[], int, int);
void initialize_memory_modules(struct memory_module[], int);
void initialize_acp(struct processor[], int);
void uniform(struct processor[], int, int);
void merge_arrays(int, int, int, struct processor[], struct processor[], struct processor[]);

void print_p (struct processor p) {
  printf("request: %i\n", p.request);
  printf("access: %i\n", p.access_counter);
  printf("priority: %i\n", p.priority);
}

/**
 * MAIN FUNCTION
 * @param[in] argc The total amount of arguments passed
 * @param[in] argv Argument vector with 2 arguments
*/
int main(int argc, char *argv[]) {
  int p = atoi(argv[1]); /* the number of processors to simulate */
  char *d = argv[2]; /* the distribution of the memory requests */
  for(int m = 1; m <= MAX_MODULES; m++) {
    S(p, m, d);
  }
}

void print_array(int array[], int length) {
  for(int i = 0; i < length; i ++) {
    printf("%i ", array[i]);
  }
  printf("\n");
}


/* FUNCTIONS */

void S(int p, int m, char *d) {
  struct processor processors[p];
  struct memory_module m_modules[m];

  initialize_memory_modules(m_modules, m);
  initialize_acp(processors, p);
  uniform(processors, p, m);
  /*for(int i = 0; i < p; i++) {
    printf("%i ", processors[i].access_counter);
  }*/
  //printf("\n");
  int selected_module;
  int f_counter;
  int u_counter;

  for(int c = 0; c < MAX_CYCLES; c++) {
    struct processor fulfilled[p];
    struct processor unfulfilled[p];
    f_counter = 0;
    u_counter = 0;
    for(int i = 0; i < p; i ++) {
      selected_module = processors[i].request;
      if(m_modules[selected_module].free == 0) {
        m_modules[selected_module].free = 1;

        processors[i].request = rand() % m;
        processors[i].granted++;
        processors[i].cumulative_average = processors[i].granted / (c + 1.0);

        fulfilled[f_counter] = processors[i];
        f_counter++;
        //granted ++;
      } else {
        processors[i].access_counter ++;
        unfulfilled[u_counter] = processors[i];
        u_counter++;
      }
    }

    merge_arrays(
      u_counter,
      f_counter,
      p,
      unfulfilled,
      fulfilled,
      processors);
    initialize_memory_modules(m_modules, m);
  }

  //printf("\n");

  double avg_cum = 0.0;
  for(int i = 0; i < p; i++) {
    //printf("Processor %i: %f\n", i, processors[i].cumulative_average);
    avg_cum += processors[i].cumulative_average;
  }
  avg_cum /= p;
  printf("%i Final: %f\n", m, avg_cum);
}


/**
 * Set all 'free' value of all memory modules in an array to 0
 * 
 * @param[in] modules Array of memory modules to be initialized
 * @param[in] m       The length of the modules array
*/
void initialize_memory_modules(struct memory_module m_modules[], int m) {
  for (int i = 0; i < m; i++) {
    m_modules[i].free = 0;
  }
}

/**
 * Initialize the access counters and granted variables for all
 * processors in an array
 * 
 * @param[in] processors Array of processors to be initialized
 * @param[in] p          The total amount of processors
*/
void initialize_acg(struct processor processors[], int p) {
  for(int i = 0; i < p; i++) {
    processors[i].access_counter = 0;
    processors[i].granted = 0.0;
  }
}

/**
 * Merge arrays for processors 'a' and 'b' and store the results in another array
 * based on the amount of processors. The 'processors' array will be replaced with
 * a new array of processors in order of unfulfilled then fulfilled processors 
 * (this will simulate order of priority in the next cycle).
 * 
 * @param[in] u Total amount of processors found in array 'a'
 * @param[in] f Total amount of processors found in array 'b'
 * @param[in] p Total amount of processors to be store in array 'processors'
 * @param[in] a Array that holds unfulfilled processors from indices 0 to u
 * @param[in] b Array that holds fulfilled processors from indices 0 to f
 * @param[in] processors Current array of processors to be replaced
*/
void merge_arrays(
  int u,
  int f,
  int p, 
  struct processor a[],
  struct processor b[],
  struct processor processors[]) {
  int j = 0;
  for(int i = 0; i < u; i ++) {
    processors[j] = a[i];
    j++;
  }

  for(int i = 0; i < f; i++) {
    processors[j] = b[i];
    j++;
  }
}

/**
 * Generate a new request for each processor in an array by
 * means of normal distribution.
 * 
 * @param[in] processors Array of processors that will generate a new request
 * @param[in] p          Total amount of processors found in the array
 * @param[in] m          Total amount of memory modules available
*/
void uniform(struct processor processors[], int p, int m) {
  srand(time(NULL)); /* use a new seed to help with randomizing */
  for(int i = 0; i < p; i++) {
    int s = rand() % m;
    processors[i].request = s; /* randomly select a memory module and assign  */
    //printf("%i ", s);
  }
  //printf("\n results in \n");
}