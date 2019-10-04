
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/perf_event.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define NUMBER_OF_EVENTS 3

/* read_format structure */
struct read_format {
  uint64_t nr;
  uint64_t time_enabled;
  uint64_t time_running;
  struct {
    uint64_t value;
    uint64_t id;
  } values[];
};

/* wrapper function for perf_event_open */
int perf_event_open( struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags )
{
  int fd;

  fd = syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);

  return fd;

}

/* main function */
int main( void )
{

  /* perf variables */
  struct perf_event_attr pe;
  uint64_t fd[NUMBER_OF_EVENTS];
  uint64_t id[NUMBER_OF_EVENTS];
  uint64_t val[NUMBER_OF_EVENTS];
  struct read_format *rf;
  char buffer[(3+2*NUMBER_OF_EVENTS)*8];
  rf = (struct read_format*) buffer;

  uint64_t etype[NUMBER_OF_EVENTS];
  uint64_t econf[NUMBER_OF_EVENTS];

  /* compute variables */
  uint64_t N = 256 * 1024 * 1024 / 4;
  int a[N];
  uint64_t i;

  /* define perf event attributes */

  /* define events */
  etype[0] = PERF_TYPE_HARDWARE; econf[0] = PERF_COUNT_HW_CPU_CYCLES;
  etype[1] = PERF_TYPE_HW_CACHE; econf[1] = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
  etype[2] = PERF_TYPE_HARDWARE; econf[2] = PERF_COUNT_HW_INSTRUCTIONS;

  /* run loop */
  for (i = 0; i < NUMBER_OF_EVENTS; i++)
  {
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = etype[i];
    pe.size = sizeof(struct perf_event_attr);
    pe.config = econf[i];
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_GROUP | PERF_FORMAT_ID;

    if (i == 0)
    {
      fd[i] = perf_event_open(&pe, 0, -1, -1, 0);
    }
    else
    {
      fd[i] = perf_event_open(&pe, 0, -1, fd[0], 0);
    }
    ioctl(fd[i], PERF_EVENT_IOC_ID, &id[i]);
  }

  /* start monitoring */
  ioctl(fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  ioctl(fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

  /* start compute */
  for (i = 0; i < N; i++)
  {
    a[i]++;
  }

  /* stop monitoring */
  ioctl(fd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);

  /* read results */
  read(fd[0], buffer, sizeof(buffer));

  for (i = 0; i < rf->nr; i++)
  {
    val[i] = rf->values[i].value;
  }

  for (i = 0; i < NUMBER_OF_EVENTS; i++)
  {
    printf("event [%ld] = %ld\n", i, val[i]);
  }

  return EXIT_SUCCESS;

}
