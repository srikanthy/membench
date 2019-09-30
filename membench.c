
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SAMPLE 10
#define CACHE_MIN (1024)
#define CACHE_MAX (16*1024*1024)
#define RTIME 0.1

int x[CACHE_MAX];

int main( int argc, char *argv[] )
{

  /* define file pointers */
  FILE *fp;

  /* read commandline arguments */
  if (argc == 1)
  {
    fprintf(stdout, "saving results in membench.csv\n");

    fp = fopen("membench.csv", "w");
  }
  else if (argc == 2)
  {
    fprintf(stdout, "saving results in %s\n", argv[1]);
    
    fp = fopen(argv[1], "w");
  }
  else
  {
    fprintf(stderr, "membench.x [output_file]\n");

    return EXIT_FAILURE;
  }

  /* write output file header */
  fprintf(fp, "size, stride, ns\n");

  /* run experiments */
  for (long csize = CACHE_MIN; csize <= CACHE_MAX; csize *= 2)
  {
    for (int stride = 1; stride <= csize/2; stride *= 2)
    {
      double sec0 = 0;
      double sec1 = 0;

      int limit = csize - stride + 1;

      /* time the loop with strided access + loop overhead */
      int steps = 0;
      double start = omp_get_wtime();
      do
      {
        for (int i = SAMPLE * stride; i != 0; i--)
        {
          for (int index = 0; index < limit; index += stride)
          {
            x[index]++;
          }
        }
        steps++;
        sec0 = omp_get_wtime() - start;
      } while (sec0 < RTIME);

      /* time just the overheads */
      int tsteps = 0;
      int temp = 0;
      start = omp_get_wtime();
      do 
      {
        for (int i = SAMPLE * stride; i != 0; i--)
        {
          for (int index = 0; index < limit; index += stride)
          {
            temp += index;
          }
        }
        tsteps++;
        sec1 = omp_get_wtime() -start;
      } while (tsteps < steps);

      /* report on the average time per read/write */
      double sec = sec0 - sec1;
      double ns_per_step = (sec*1.e9)/steps;
      double reads_per_step = SAMPLE * stride * ((limit-1.0)/stride + 1.0);

      fprintf(fp, "%lu, %lu, %f\n", csize * sizeof(int), stride * sizeof(int), ns_per_step/reads_per_step);

    }
  }

  return EXIT_SUCCESS;

}
