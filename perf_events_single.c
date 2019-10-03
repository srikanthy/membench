
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <asm/unistd.h>

static long perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{

  int ret;

  ret = syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);

  return ret;

}

int main( void )
{

  struct perf_event_attr pe;
  long long count;
  int fd;

  /* data array */
  int N = 33553332;
  int a[N];
  int i;
  int j;


  memset(&pe, 0, sizeof(struct perf_event_attr));
  printf("size = %ld\n", sizeof(struct perf_event_attr));

  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = PERF_COUNT_HW_CACHE_MISSES;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;
  
  fd = perf_event_open(&pe, 0, -1, -1, 0);
  if( fd == -1)
  {
    fprintf(stderr, "Error opening leader %llx\n", pe.config);
    exit(EXIT_FAILURE);
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

  for(j = 0; j < 1024; j++)
    for (i = 0; i<N; i+=1024)
      a[i]++;

  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
  read(fd, &count, sizeof(long long));

  printf("%lld cache misses\n", count);

  close(fd);

  return EXIT_SUCCESS;

}
