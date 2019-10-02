
/* -- membench : Memory Microbenchmark --
 *
 * A simple memory benchmark to understand the affect 
 * of cache memory on strided access
 *
 * membench:
 * for array A of length L from 4KB to 64MB by 2x
 *   for stride s from 4 bytes (1 word) to L/2 by 2x
 *     time following loop
 *     for i from 1 to niterations
 *       for index 0 to L by s
 *         load A[i] from memory
 *
 * references: 1. {Lecture 2, COMP 422, Parallel Computing, Spring 2008,
 *                 https://www.cs.rice.edu/~vs3/comp422/}
 *             2. {Lecture 2, CS 5220, Applications of Parallel Computers,
 *                 https://www.cs.cornell.edu/~bindel/class/cs5220-s10/index.html}
 *
 * author: srikanth yalavarthi
 * last modified : 2019-10-02
 * url: https://github.com/srikanthy/membench.git
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* define macros*/
#define HIRES_CLOCK

#ifndef MIN_ARRAY_SIZE
#define MIN_ARRAY_SIZE 4          // size in kB
#endif

#ifndef MAX_ARRAY_SIZE
#define MAX_ARRAY_SIZE (64*1024)  // size in kB
#endif

#ifndef NITERATIONS
#define NITERATIONS 10            // number of iteratons
#endif

#ifndef SAMPLE_INTERVAL
#define SAMPLE_INTERVAL 0.5       // max time for a stride
#endif

#ifdef HIRES_CLOCK
#define MEMBENCH_CLOCK nanoclock
#define MULTIPLIER 1
#else
#define MEMBENCH_CLOCK microclock
#define MULTIPLIER 1000
#endif

/* function prototypes */
long long int nanoclock(void);
long long  int microclock(void);

int main( int argc, char *argv[] )
{

  /* io variables */
  char fname[32];
  FILE *fp;

  /* array and cache sizes */
  int min_array_size = MIN_ARRAY_SIZE * 1024 / sizeof(int);  // KB -> NINTS
  int max_array_size = MAX_ARRAY_SIZE * 1024 / sizeof(int);  // KB -> NINTS

  /* local variables */
  int x[max_array_size];
  int array_size;
  int stride;
  int idx;
  int ilimit;
  int niterations = NITERATIONS;
  int i;
  int l1steps;
  int l2steps;
  int tvar = 0;

  /* timer and stats variables */
  long long int l1start, l1time;
  long long int l2start, l2time;
  double runtime;
  double steptime;
  double stepreads;
  double access_time;

  /* read commandline arguments */
  if (argc == 1)
  {
    strcpy(fname, "membench.csv");
  }
  else if (argc == 2)
  {
    strcpy(fname, argv[1]);
  }
  else
  {
    printf("Usage: ./membench.x [output_filename]\n");
    return EXIT_FAILURE;
  }

  /* open file for writing */
  fp = fopen(fname, "w");
  fprintf(stdout, "membench: writing output to %s\n", fname);

  /* write header */
  fprintf(fp, "size,stride,time\n");

  /* membench algorithm -- start */
  /* start loops */
  for (array_size = min_array_size;  array_size <= max_array_size; array_size *= 2)
  {
    for (stride = 1; stride <= array_size/2; stride *= 2)
    {

      ilimit = array_size - stride + 1;

      /* loop 1 - strided access + overhead */
      l1steps = 0;
      l1start = MEMBENCH_CLOCK();

      do
      {
        for (i = 1; i <= niterations * stride; i++)
        {
          for (idx = 0; idx < ilimit; idx += stride)
          {
            x[idx]++;
          }
        }

        l1steps++;
        l1time = MEMBENCH_CLOCK() - l1start;

      } while (l1time < SAMPLE_INTERVAL);

      /* loop 2 - overhead */
      l2steps = 0;
      l2start = MEMBENCH_CLOCK();

      do
      {

        for (i = 1; i <= niterations * stride; i++)
        {
          for (idx = 0; idx < ilimit; idx += stride)
          {
            tvar += idx;
          }
        }

        l2steps++;
        l2time = MEMBENCH_CLOCK() - l2start;

      } while (l2steps < l1steps);

      /* write timings */
      runtime = (double)(l1time - l2time);
      steptime = MULTIPLIER * runtime/l1steps;
      stepreads = niterations * stride * ((ilimit - 1.0)/stride + 1.0);
      access_time = steptime/stepreads;

      fprintf(fp, "%lu,%lu,%f\n", array_size * sizeof(int), stride * sizeof(int), access_time);
      fflush(fp);

    }
  }
  /* membench algorithm -- end */

  /* close file */
  fclose(fp);

  return EXIT_SUCCESS;

}

/* nanosecond precision clock */

#include <time.h>

long long int nanoclock(void)
{

  struct timespec tv;

  clock_gettime(CLOCK_REALTIME, &tv);

  return (1000000000 * tv.tv_sec + tv.tv_nsec);

}

/* microsecond precision clock */

#include <sys/time.h>

long long int microclock(void)
{

  struct timeval tv;

  gettimeofday(&tv, NULL);

  return (1000000 * tv.tv_sec + tv.tv_usec);

}
