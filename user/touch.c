#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i, fd;

  if(argc < 2){
    fprintf(2, "usage: touch file ...\n");
    exit(1);
  }

  for(i = 1; i < argc; i++){
    if((fd = open(argv[i], O_CREATE | O_WRONLY | O_TRUNC)) < 0){
      fprintf(2, "touch: %s failed to create\n", argv[i]);
      continue;
    }
    close(fd);
  }
  exit(0);
}
