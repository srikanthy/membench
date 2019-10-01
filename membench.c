

#include <stdio.h>
#include <stdlib.h>

/* function prototypes */
long long  int microclock();
long long int nanoclock();

int main( int argc, char *argv[] )
{

  return EXIT_SUCCESS;

}

/* microsecond precision clock */

#include <sys/time.h>

long long int microclock()
{

  struct timeval tv;

  gettimeofday(&tv, NULL);

  return (1000000 * tv.tv_sec + tv.tv_usec);

}


/* nanosecond precision clock */

#include <time.h>

long long int nanosecond()
{

  struct timespec tv;

  clock_gettime(CLOCK_REALTIME, &tv);

  return (1000000000 * tv.tv_sec + tv.tv_nsec);

}
