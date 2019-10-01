
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* define cache sizes in kB */
#ifndef L1_CACHE
#define L1_CACHE 32
#endif

#ifndef L2_CACHE
#define L2_CACHE 256
#endif

#ifndef L3_CACHE
#define L3_CACHE 6144
#endif

#define TOTAL_CACHE (L1_CACHE+L2_CACHE+L3_CACHE)

long long int microclock();

int main( int argc, char *argv[] )
{

  /* filenames */
  char fname[32];

  /* file handles */
  FILE *fp;

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

  /* print cache information */
  fprintf(stdout, "L1 Cache : %d kB\n", L1_CACHE);
  fprintf(stdout, "L2 Cache : %d kB\n", L2_CACHE);
  fprintf(stdout, "L3 Cache : %d kB\n", L3_CACHE);

  /* open output file and write header */
  fp = fopen(fname, "w");
  fprintf(stdout, "saving output to %s\n", fname);
  
  /* membench */

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
