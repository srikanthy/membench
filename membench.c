

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* define macros*/
#define MIN_ARRAY_SIZE 4          // size in kB
#define MAX_ARRAY_SIZE (64*1024)  // size in kB
#define NITERATIONS 10            // number of iteratons
#define SAMPLE_INTERVAL 0.5       // max time for a stride

/* function prototypes */
#ifdef __linux__
#define MEMBENCH_CLOCK nanoclock
#define MULTIPLIER 1
long long int nanoclock();
#else
#define MEMBENCH_CLOCK microclock
#define MULTIPLIER 1000
long long  int microclock();
#endif

int main( int argc, char *argv[] )
{

  /* io variables */
  char fname[32];
  FILE *fp;

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
  fprintf(fp, "size, stride, ns\n");

  /* membench algorithm -- start */

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

  /* timer variables */
  long long int l1start, l1time;
  long long int l2start, l2time;

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

#ifdef DEVEL
      printf("l1time = %lld, l2time = %lld, diff = %lld\n", l1time, l2time, l1time-l2time);
#endif

      /* write timings */
      double runtime = (double)(l1time - l2time);
      double steptime = MULTIPLIER * runtime/l1steps;
      double stepreads = niterations * stride * ((ilimit - 1.0)/stride + 1.0);
      double access_time = steptime/stepreads;

      fprintf(fp, "%lu, %lu, %f\n", array_size * sizeof(int), stride * sizeof(int), access_time);
      fflush(fp);

    }
  }

  /* membench algorithm -- end */

  /* close file */
  fclose(fp);

  return EXIT_SUCCESS;

}

#ifdef __linux__
/* nanosecond precision clock */

#include <time.h>

long long int nanoclock()
{

  struct timespec tv;

  clock_gettime(CLOCK_REALTIME, &tv);

  return (1000000000 * tv.tv_sec + tv.tv_nsec);

}

#else

/* microsecond precision clock */

#include <sys/time.h>

long long int microclock()
{

  struct timeval tv;

  gettimeofday(&tv, NULL);

  return (1000000 * tv.tv_sec + tv.tv_usec);

}

#endif
