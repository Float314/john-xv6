#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: nice increment command [args...]\n");
    exit(1);
  }

  int inc = atoi(argv[1]);
  if(nice(inc) < 0){
    fprintf(2, "nice failed\n");
    exit(1);
  }

  exec(argv[2], &argv[2]);
  fprintf(2, "exec %s failed\n", argv[2]);
  exit(1);
}
