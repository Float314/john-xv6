#include "kernel/types.h"
#include "user/user.h"

int
rename(const char *old, const char *new)
{
  if(link(old, new) < 0)
    return -1;
  return unlink(old);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "usage: mv old new\n");
    exit(1);
  }

  if(rename(argv[1], argv[2]) < 0){
    fprintf(2, "mv: failed to rename %s to %s\n", argv[1], argv[2]);
    exit(1);
  }

  exit(0);
}
