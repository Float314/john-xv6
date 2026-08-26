#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: renice priority pid\n");
    exit(1);
  }

  int priority = atoi(argv[1]);
  int pid = atoi(argv[2]);

  if(renice(pid, priority) < 0){
    fprintf(2, "renice failed\n");
    exit(1);
  }

  exit(0);
}
