
#include <stdlib.h>
#include <stdio.h>
#include <linux/perf_event.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <string.h>
#include <sys/ioctl.h>

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
  long long int val;

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

  /* do some work */
  for (i = 0; i < N; i++)
  {
    A[i]++;
  }

  /* disable the event*/
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

  /* read results */
  read(fd, &val, sizeof(long long int));

  /* close */
  close(fd);

  /* print result */
  fprintf(stdout, "CPU Cyles = %lld\n", val);

  return EXIT_SUCCESS;

}
