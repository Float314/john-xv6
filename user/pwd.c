#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Tiny getcwd shim for xv6.
// traverses up the directory tree to build the path.
char *
getcwd(char *buf, int len)
{
  char path[128];
  char comp[DIRSIZ + 1];
  struct stat st_cur, st_up;
  int fd, n, off, found;
  
  path[0] = '\0';

  for(int depth = 0; depth < 32; depth++){
    if(stat(".", &st_cur) < 0) return 0;
    
    if((fd = open("..", O_RDONLY)) < 0) return 0;
    if(fstat(fd, &st_up) < 0){
      close(fd);
      return 0;
    }

    if(st_up.ino == st_cur.ino){ // roooooooooooot reached
      close(fd);
      break;
    }

    found = 0;
    struct dirent de;
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == st_cur.ino){
        memcpy(comp, de.name, DIRSIZ);
        comp[DIRSIZ] = '\0';
        found = 1;
        break;
      }
    }
    close(fd);

    if(!found) return 0;

    // Prepend component to path
    int comp_len = strlen(comp);
    int path_len = strlen(path);
    if(path_len + comp_len + 2 > sizeof(path)) return 0;

    memmove(path + comp_len + 1, path, path_len + 1);
    path[0] = '/';
    memcpy(path + 1, comp, comp_len);

    if(chdir("..") < 0) return 0;
  }

  chdir(path[0] != '\0' ? path : "/");

  if(strlen(path) + 1 > len) return 0;
  strcpy(buf, path[0] != '\0' ? path : "/");
  return buf;
}

int
main(int argc, char *argv[])
{
  char buf[128];
  if(getcwd(buf, sizeof(buf)) == 0){
    fprintf(2, "pwd: failed\n");
    exit(1);
  }
  printf("%s\n", buf);
  exit(0);
}
