
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/perf_event.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>

#define PERF_GROUP_EVENTS

/* read_format structure */
#ifdef PERF_GROUP_EVENTS
struct read_format {
  __u64 nr;
  __u64 time_enabled;
  __u64 time_running;
  struct {
    __u64 value;
    __u64 id;
  } values[];
};
#else
struct read_format {
  __u64 value;
//__u64 time_enabled;
//__u64 time_running;
  __u64 id;
};
#endif

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
  __u64 fd1, fd2;
  __u64 id1, id2;
  __u64 val1, val2;
  __u64 en_time;
  __u64 rn_time;
#ifdef PERF_GROUP_EVENTS
  struct read_format *rf;
  char buffer[4096];
  rf = (struct read_format*) buffer;
#else
  struct read_format rf1, rf2;
#endif

  /* compute variables */
  __u64 N = 256 * 1024 * 1024 / 4;
  int a[N];
  __u64 i;

  /* define perf event attributes */

  // PERF_COUNT_HW_CPU_CYCLES
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = PERF_COUNT_HW_CPU_CYCLES;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;
#ifdef PERF_GROUP_EVENTS
//pe.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_GROUP | PERF_FORMAT_ID;
#else
  pe.read_format = PERF_FORMAT_ID;
#endif
  fd1 = perf_event_open(&pe, 0, -1, -1, 0);
  ioctl(fd1, PERF_EVENT_IOC_ID, &id1);

  // PERF_COUNT_HW_CACHE
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HW_CACHE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
  pe.disabled = 0;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;
#ifdef PERF_GROUP_EVENTS
//pe.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_GROUP | PERF_FORMAT_ID;
#else
  pe.read_format = PERF_FORMAT_ID;
#endif
  fd2 = perf_event_open(&pe, 0, -1, fd1, 0);
  ioctl(fd2, PERF_EVENT_IOC_ID, &id2);

  /* start monitoring */
  ioctl(fd1, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  ioctl(fd1, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

  /* start compute */
  for (i = 0; i < N; i++)
  {
    a[i]++;
  }

  /* stop monitoring */
  ioctl(fd1, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);

  /* read results */
#ifdef PERF_GROUP_EVENTS
  read(fd1, buffer, sizeof(buffer));

  en_time = rf->time_enabled;
  rn_time = rf->time_running;

  for (i = 0; i < rf->nr; i++)
  {
    if (rf->values[i].id == id1)
    {
      val1 = rf->values[i].value;
    }
    else if (rf->values[i].id == id2)
    {
      val2 = rf->values[i].value;
    }
  }
#else
  read(fd1, &rf1, sizeof(rf1));
  read(fd2, &rf2, sizeof(rf2));

  val1 = rf1.value;
  val2 = rf2.value;
#endif

  printf("cpu cycles = %lld in %lld\n", val1, en_time);
  printf("cache misses = %lld in %lld\n", val2, rn_time);


  return EXIT_SUCCESS;

}
