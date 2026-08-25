#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"

const char *logo = R"md(
                                         
  ,--.                            ,--.  
  `--',-----.,--.  ,--.,--.  ,--./  .'  
  ,--.'-----' \  `'  /  \  `'  /|  .-.  
  |  |        /  /.  \   \    / \   o | 
.'-'  /       '--'  '--'   `--'   `---'  
'---'                                   
)md";

static int
countfiles(void)
{
  int fd, count;
  struct dirent de;

  fd = open("/", O_RDONLY);
  if (fd < 0)
    return 0;
  count = 0;
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum != 0)
      count++;
  }
  close(fd);
  return count;
}

int
main(int argc, char *argv[])
{
  int ticks, secs, mins, hours, pkgs;
  uint64 mem;

  printf("%s", logo);

  ticks = uptime();
  secs = ticks / 10;
  mins = secs / 60;
  hours = mins / 60;
  secs %= 60;
  mins %= 60;

  mem = (uint64)sbrk(0);
  pkgs = countfiles();

  printf("  root@qemu\n");
  printf("  -----------\n");
  printf("  OS:       johnxv6 riscv64\n");
  printf("  Host:     QEMU RISC-V virt\n");
  printf("  Kernel:   xv6-riscv\n");
  if (hours > 0)
    printf("  Uptime:   %d hours, %d mins, %d secs\n", hours, mins, secs);
  else if (mins > 0)
    printf("  Uptime:   %d mins, %d secs\n", mins, secs);
  else
    printf("  Uptime:   %d secs\n", secs);
  printf("  Packages: %d (files)\n", pkgs);
  printf("  Shell:    sh\n");
  printf("  Terminal: console\n");
  printf("  CPU:      rv64gc (3)\n");
  printf("  Memory:   %dKB / 128MiB\n", (int)(mem / 1024));

  exit(0);
}
