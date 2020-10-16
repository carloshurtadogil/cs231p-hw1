#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* CONSTANTS */
#define MAX_CYCLES 1000000 /* maximum amount of memory cycles */
#define MIN_CYLCES 5000 /* minimum cycles before we can short circut by delta */
#define MAX_MODULES 2048 /* maximum amount of memory modules for the simulation */
#define DELTA 0.0001

/* STRUCTS */

/** 
 * Defines a memory module and its status as either
 * free (0) or not (1)
*/
typedef struct memory_module {
  int free;
} memory_module;

/**
 * Defines the anatomy of a mock processor for
 * this assignment
*/
typedef struct processor {
  int request, access_counter;
  double cumulative_average;
  double granted; // the amount of times a processor was granted access to its requested memory module
} processor;

/* FUNCTION DECLARATIONS */
void S(int, int, char);
void initialize_memory_modules(memory_module[], int);
void initialize_acg(processor[], int);
void uniform(processor[], int, int);
double get_normal_value(int, double);
void merge_arrays(int, int, int, processor[],  processor[], processor[]);
 
/* FUNCTIONS */

/**
 * MAIN FUNCTION
 * @param[in] argc The total amount of arguments passed
 * @param[in] argv Argument vector with 2 arguments
*/
int main(int argc, char *argv[]) {
  int p = atoi(argv[1]); /* the number of processors to simulate */
  char d = argv[2][0]; /* the distribution of the memory requests */
  for(int m = 1; m <= MAX_MODULES; m++) {
    S(p, m, d);
  }
}

/**
 * Function to run a simulation a given amount of processors to generate
 * and carry out requests to a given amount of memory modules for a 
 * given amount of cycles. Prints out the last arithmetic average of
 * all processors' time cumulative averages.
 * 
 * @param[in] p The amount of processors that will generate a request
 * @param[in] m The amount of memory modules available for the simulation
 * @param[in] d Char that specifies the distribution of memory requests
*/
void S(int p, int m, char d) {
  processor processors[p]; // create array of processors
  memory_module m_modules[m]; // create array of memory modules

  initialize_memory_modules(m_modules, m); // initialize each memory module
  initialize_acg(processors, p); // initialize  each processor
  uniform(processors, p, m); // set the request value of each processor via uniform distribution
  
  int selected_module; // the module that is selected for each processor
  int f_counter; // index counter for processors that are to be placed in fulfilled array
  int u_counter; // index counter for processors that are to be placed in unfullfilled array
  double w_bar = 0.0; // arithmetic average of all processors' time cumulative averages
  double prev_w_bar = 0.0; // w_bar of the cycle c - 1

  for(int c = 0; c < MAX_CYCLES; c++) { // for every cycle
    processor fulfilled[p]; // array of processors that do not have to wait this cycle (will assume lower priority for next cycle)
    processor unfulfilled[p]; // array of processors that have to wait this cycle (will assume higher priority for next cycle)
    f_counter = 0;
    u_counter = 0;

    for(int i = 0; i < p; i ++) { // for every processor in processors array
      selected_module = processors[i].request; // set index of p[i]'s requested memory module

      if(m_modules[selected_module].free == 0) { // if memory module is available
        m_modules[selected_module].free = 1; // flag memory module as unavailable

        // distribute the processor's next request based on value d
        if (d == 'u') // uniform distribution
          processors[i].request = rand() % m;
        else { // normal distribution 
          int prev = processors[i].request;
          double normal_value = get_normal_value(prev, m / 6.0);
          processors[i].request = abs((int)round(normal_value) % m);
        }
        
        processors[i].granted++;
        processors[i].cumulative_average = (c + 1) / processors[i].granted; // calculate the cumulative average for p for this cycle

        fulfilled[f_counter] = processors[i]; // add processor to fulfilled array
        f_counter++; // increase counter
      } else { // if memory module was not available this cycle
        processors[i].access_counter++; // increase the amount of cycles processor p had to wait
        unfulfilled[u_counter] = processors[i]; // add processor to unfulfilled array
        u_counter++; //increase counter
      }
    }

    w_bar = 0.0; // reset w_bar
    for(int i = 0; i < p; i++) {
      w_bar += processors[i].cumulative_average; // calculate the cumulative averages for this cycle
    }
    w_bar /= p;
    --w_bar; 

    /**
     * end the simulation early when the absolute value of the difference of 1
     * and (prev_w_bar and w_bar) is less than tolerance DELTA
    */
    if (c > MIN_CYLCES) {
      double ratio = prev_w_bar / w_bar;
      double diff = fabs(1.0 - ratio);
      if (diff < DELTA)
        break;
    }
    prev_w_bar = w_bar;

    // merge arrays unfulfilled and fulfilled
    merge_arrays(
      u_counter,
      f_counter,
      p,
      unfulfilled, // higher priority
      fulfilled, // lower priority
      processors);
    initialize_memory_modules(m_modules, m); // reset memory modules' 'free' attribute to 0
  }

  printf("%0.4f\n", w_bar); // print out the result of the last w_bar when simulation ended
}


/**
 * Set all 'free' value of all memory modules in an array to 0
 * 
 * @param[in] modules Array of memory modules to be initialized
 * @param[in] m       The length of the modules array
*/
void initialize_memory_modules(memory_module m_modules[], int m) {
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
void initialize_acg(processor processors[], int p) {
  for(int i = 0; i < p; i++) {
    processors[i].access_counter = 0;
    processors[i].granted = 0.0;
    processors[i].cumulative_average = 0.0;
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
  processor a[],
  processor b[],
  processor processors[]) {
  int j = 0;
  if (u + f == p) {
    for(int i = 0; i < u; i ++) {
      processors[j] = a[i];
      j++;
    }

    for(int i = 0; i < f; i++) {
      processors[j] = b[i];
      j++;
    }
  } else {
    exit(1);
  }
}

/**
 * Generate a new request for each processor in an array by
 * means of normal distribution. This function is an implemntation of hte
 * 
 * @param[in] processors Array of processors that will generate a new request
 * @param[in] p          Total amount of processors found in the array
 * @param[in] m          Total amount of memory modules available
*/
void uniform(processor processors[], int p, int m) {
  srand(time(NULL)); /* use a new seed to help with randomizing */
  for(int i = 0; i < p; i++) {
    processors[i].request = rand() % m;
  }
}

/**
 * Generate's a value from a specified normal distribution. 
 * This is an implementation of the Marsaglia polar method. 
 * Since this algorithm generates bye construction two values, it is 
 * reentrant and stores the second of the two for subsequent calls.
 * 
 * @param[in] mean    The mean of the distribution
 * @param[in] std_dev The standard deviation of the distribution 
 */
double get_normal_value(int mean, double std_dev) {
  static double value_1, value_2, S;
  static int phase = 0;
  double X;
  
  if (phase == 0) {
    do {
      double d1 = (double)rand() / RAND_MAX;
      double d2 = (double)rand() / RAND_MAX;
      value_1 = 2 * d1 - 1;
      value_2 = 2 * d2 - 1;
      S = value_1 * value_1 + value_2 * value_2;
    } while(S >= 1 || S == 0);
    X = value_1 * sqrt(-2 * log(S) / S);
  } else 
    X = value_2 * sqrt(-2 * log(S) / S);

  phase = 1 - phase;

  return X * std_dev + mean;
}