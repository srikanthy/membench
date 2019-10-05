
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
#define ENABLE_PERF_COUNTERS
#define PERF_EVENTS_COUNT 3

#ifndef MIN_ARRAY_SIZE
#define MIN_ARRAY_SIZE 4          // size in kB
#endif

#ifndef MAX_ARRAY_SIZE
#define MAX_ARRAY_SIZE (6*1024)   // size in kB
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

#ifdef ENABLE_PERF_COUNTERS
/* file headers */
#include <stdint.h>
#include <linux/perf_event.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>

/* define read_format structure */
struct read_format {
  uint64_t nr;
  uint64_t time_enabled;
  uint64_t time_running;
  struct {
    uint64_t value;
    uint64_t id;
  } values[];
};

/* wrapper function for per_event_open */
int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned flags)
{

  int fd;

  fd = syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);

  return fd;

}
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

#ifdef ENABLE_PERF_COUNTERS
  /* perf variables */
  int nevents = PERF_EVENTS_COUNT;
  struct perf_event_attr pe_attr;
  uint64_t fd[nevents];
  uint64_t id[nevents];
  uint64_t val[nevents];
  char ecode[nevents][32];
  struct read_format *rf;
  char buffer[(3 + 2 * nevents) * 8];
  rf = (struct read_format*) buffer;

  uint64_t etype[nevents];
  uint64_t econf[nevents];

  /* define perf events */
  strcpy(ecode[0], "CPU Cycles");      etype[0] = PERF_TYPE_HARDWARE; econf[0] = PERF_COUNT_HW_CPU_CYCLES;
  strcpy(ecode[1], "Instructions");    etype[1] = PERF_TYPE_HARDWARE; econf[1] = PERF_COUNT_HW_INSTRUCTIONS;
  strcpy(ecode[2], "L1 Cache Misses"); etype[2] = PERF_TYPE_HW_CACHE; econf[2] = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
#endif

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


#ifdef ENABLE_PERF_COUNTERS
  /* open perf events */
  for (i = 0; i < nevents; i++)
  {
    memset(&pe_attr, 0, sizeof(pe_attr));
    pe_attr.type = etype[i];
    pe_attr.size = sizeof(struct perf_event_attr);
    pe_attr.config = econf[i];
    pe_attr.disabled = 1;
    pe_attr.exclude_kernel = 1;
    pe_attr.exclude_hv = 1;
    pe_attr.read_format = PERF_FORMAT_ID | PERF_FORMAT_GROUP | PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;

    if (i == 0)
    {
      fd[i] = perf_event_open(&pe_attr, 0, -1, -1, 0);
    }
    else
    {
      fd[i] = perf_event_open(&pe_attr, 0, -1, fd[0], 0);
    }

    ioctl(fd[i], PERF_EVENT_IOC_ID, &id[i]);
  }

  /* write header */
  fprintf(fp, "Size,Stride,Time (ns)");

  for (i = 0; i < nevents; i++)
  {
    fprintf(fp, ",%s", ecode[i]);
  }

  fprintf(fp, "\n");
#else

  /* write header */
  fprintf(fp, "Size,Stride,Time (ns)\n");
#endif

  /* membench algorithm -- start */
  /* start loops */
  for (array_size = min_array_size;  array_size <= max_array_size; array_size *= 2)
  {
    for (stride = 1; stride <= array_size/2; stride *= 2)
    {

      ilimit = array_size - stride + 1;
      l1steps = 0;
      l1start = MEMBENCH_CLOCK();

      /* start monitoring */
      ioctl(fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
      ioctl(fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

      /* loop 1 - strided access + overhead */
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

      /* stop monitoring */
      ioctl(fd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);

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

#ifdef ENABLE_PERF_COUNTERS
      read(fd[0], buffer, sizeof(buffer));

      for (i = 0; i < nevents; i++)
      {
        if( rf->values[i].id == id[i] )
         {
           val[i] = rf->values[i].value;
         }
      }

      fprintf(fp, "%lu,%lu,%f", array_size * sizeof(int), stride * sizeof(int), access_time);

      for (i = 0; i < nevents; i++)
      {
        fprintf(fp, ",%lu", val[i]);
      }

      fprintf(fp, "\n");
#else
      fprintf(fp, "%lu,%lu,%f\n", array_size * sizeof(int), stride * sizeof(int), access_time);
#endif
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
