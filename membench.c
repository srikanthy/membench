
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KB_CACHE_MIN 1
#define KB_CACHE_MAX 1024

#define CACHE_MIN (KB_CACHE_MIN * 1024)
#define CACHE_MAX (KB_CACHE_MAX * 1024)

#define NITERATIONS 10

#define RTIME (100000)

long long int microclock();

int main( int argc, char *argv[] )
{

  /* filenames */
  char fname[32];

  /* file handles */
  FILE *fp;

  /* data arrays */
  int x[CACHE_MAX];

  /* loop variables */
  int length;
  int stride;
  int idx;
  int limit;
  int l1steps, l2steps;
  int i;
  int y;

  /* timer variables */
  long long int stime;
  long long int l1time;
  long long int l2time;

  /* read commandline arguments */
  if (argc == 1)
  {
    strcpy(fname, "membench.csv");
  }
  else if (argc ==2)
  {
    if (strlen(argv[1]) >= sizeof(fname))
    {
      fprintf(stderr, "filename too long ... exit\n");
      return EXIT_FAILURE;
    }
    strcpy(fname, argv[1]);
  }
  else
  {
    fprintf(stderr, "./membench [output_file]\n");
    return EXIT_FAILURE;
  }

  /* open output file and write header */
  fp = fopen(fname, "w");
  fprintf(stdout, "saving output to %s\n", fname);

  /* membench */
  for (length = CACHE_MIN; length <= CACHE_MAX; length *= 2)
  {
    for (stride = 1; stride < length/2; stride *= 2)
    {
      limit = length - stride + 1;

      l1steps = 0;
      stime = microclock();
      do
      {
        for (i = 1; i <= stride * NITERATIONS; i++)
        {
          for (idx = 0; idx < limit; idx += stride)
          {
//          printf("length = %d, stride = %d, idx = %d\n", length, stride, idx);
            x[idx]++;
          }
        }

        l1steps++;
        l1time = microclock() - stime;

      } while (l1time < RTIME);

      l2steps = 0;
      stime = microclock();
      y = 0;
      do
      {
        for (i = 1; i <= stride * NITERATIONS; i++)
        {
          for (idx = 0; idx < limit; idx += stride)
          {
//          printf("length = %d, stride = %d, idx = %d\n", length, stride, idx);
            y += idx;
          }
        }

        l2steps++;
        l2time = microclock() - stime;

      } while (l2steps < l1steps);

      printf("length = %d, stride = %d\n", length, stride);

      /* report timings */
      double rtime = l1time - l2time;
      double ns_per_step = 1000.0 * rtime / l1steps;
      double reads_per_step = NITERATIONS * stride * ((limit-1.0)/stride+1.0);
      fprintf(fp, "%ld, %ld, %f\n", length *sizeof(int), stride* sizeof(int), ns_per_step/reads_per_step);
    }
  }

  /* end of membench */

  /* close the file */
  fclose(fp);

  return EXIT_SUCCESS;

  /* end of main */

}

/* microsecond precision clock */

#include <sys/time.h>

long long int microclock()
{

  struct timeval tv;

  gettimeofday(&tv, NULL);

  return (1000000 * tv.tv_sec + tv.tv_usec);

  /* end of microclock */

}
