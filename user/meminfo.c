#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int free_bytes = meminfo();
  if (free_bytes < 0) {
    fprintf(2, "meminfo failed\n");
    exit(1);
  }

  printf("Free RAM: %d bytes (%d KB, %d MB)\n", free_bytes, free_bytes / 1024, free_bytes / (1024 * 1024));
  exit(0);
}
