
#include <stdlib.h>
#include <stdio.h>
#include <linux/perf_event.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdint.h>

#undef PERF_ENABLE_READ_FORMAT
#undef PERF_ENABLE_EVENT_TIMES

#ifdef PERF_ENABLE_READ_FORMAT
/* read format structure - no group format */
struct read_format {
  uint64_t value;
#ifdef PERF_ENABLE_EVENT_TIMES
  uint64_t time_enabled;
  uint64_t time_running;
#endif
  uint64_t id;
};
#endif

/* wrapper function for perf_event_open */
int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{

  int fd;

  fd = syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);

  return fd;

}

int main( void )
{

  /* perf variables */
  struct perf_event_attr pe_attr;
  int fd;
  uint64_t id;
#ifdef PERF_ENABLE_READ_FORMAT
  struct read_format rf;
#else
  long long int val;
#endif

  /* data array */
  int N = 4 * 1024 *1024 / 4;
  int A[N];
  int i;

  /* define perf event */
  memset(&pe_attr, 0, sizeof(struct perf_event_attr));
  pe_attr.type = PERF_TYPE_HARDWARE;
  pe_attr.size = sizeof(struct perf_event_attr);
  pe_attr.config = PERF_COUNT_HW_CPU_CYCLES;
  pe_attr.disabled = 1;
  pe_attr.read_format = PERF_FORMAT_ID;
  pe_attr.exclude_kernel = 1;
  pe_attr.exclude_hv = 1;

  /* open perf event */
  fd = perf_event_open(&pe_attr, 0, -1, -1, 0);
  if (fd == -1)
  {
    fprintf(stderr, "error opening leader %lld\n", pe_attr.config);
    exit(EXIT_FAILURE);
  }

  /* reset and enable the event */
  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(fd, PERF_EVENT_IOC_ID, &id);

  /* do some work */
  for (i = 0; i < N; i++)
  {
    A[i]++;
  }

  /* disable the event*/
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

  /* read results */
#ifdef PERF_ENABLE_READ_FORMAT
  read(fd, &rf, sizeof(rf));

  /* print result */
  fprintf(stdout, "CPU Cyles = %ld\n", rf.value);
  fprintf(stdout, "id = %ld\n", rf.id);
  fprintf(stdout, "id = %ld\n", id);

#else
  read(fd, &val, sizeof(long long int));

  /* print result */
  fprintf(stdout, "CPU Cyles = %lld\n", val);

#endif

  /* close */
  close(fd);

  return EXIT_SUCCESS;

}
