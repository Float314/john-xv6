#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"
// oh HELL NAH PLEASE DONT ABBREVIATE CYBERPUNK
void
copy(char *src, char *dst)
{
  int rfd, wfd;
  int n;
  char buf[512];
  struct stat st_src, st_dst;

  if((rfd = open(src, O_RDONLY)) < 0){
    fprintf(2, "cp: cannot open %s\n", src);
    return;
  }

  if(fstat(rfd, &st_src) < 0){
    fprintf(2, "cp: cannot stat %s\n", src);
    close(rfd);
    return;
  }

  if(st_src.type == T_DIR){
    // recursive copy
    if(mkdir(dst) < 0){
      if(stat(dst, &st_dst) < 0 || st_dst.type != T_DIR){
        fprintf(2, "cp: cannot create directory %s\n", dst);
        close(rfd);
        return;
      }
    }

    char buf_src[512], buf_dst[512];
    char *p_src, *p_dst;
    struct dirent de;

    if(strlen(src) + 1 + DIRSIZ + 1 > sizeof(buf_src) ||
       strlen(dst) + 1 + DIRSIZ + 1 > sizeof(buf_dst)){
      fprintf(2, "cp: path too long\n");
      close(rfd);
      return;
    }

    strcpy(buf_src, src);
    p_src = buf_src + strlen(buf_src);
    if(p_src > buf_src && *(p_src-1) != '/') *p_src++ = '/';

    strcpy(buf_dst, dst);
    p_dst = buf_dst + strlen(buf_dst);
    if(p_dst > buf_dst && *(p_dst-1) != '/') *p_dst++ = '/';

    while(read(rfd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      
      memmove(p_src, de.name, DIRSIZ);
      p_src[DIRSIZ] = 0;
      memmove(p_dst, de.name, DIRSIZ);
      p_dst[DIRSIZ] = 0;

      copy(buf_src, buf_dst);
    }
    close(rfd);
    return;
  }

  // file copy
  if((wfd = open(dst, O_CREATE | O_WRONLY | O_TRUNC)) < 0){
    fprintf(2, "cp: cannot open %s\n", dst);
    close(rfd);
    return;
  }

  while((n = read(rfd, buf, sizeof(buf))) > 0){
    if(write(wfd, buf, n) != n){
      fprintf(2, "cp: write error\n");
      break;
    }
  }

  close(rfd);
  close(wfd);
}

char*
get_basename(char *path)
{
  char *p;
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

int
main(int argc, char *argv[])
{
  struct stat st;
  if(argc < 3){
    fprintf(2, "usage: cp src ... dest\n");
    exit(1);
  }

  char *dest = argv[argc-1];
  int dest_is_dir = 0;
  if(stat(dest, &st) == 0 && st.type == T_DIR){
    dest_is_dir = 1;
  }

  if(argc > 3 && !dest_is_dir){
    fprintf(2, "cp: target %s is not a directory\n", dest);
    exit(1);
  }

  for(int i = 1; i < argc-1; i++){
    char target[512];
    if(dest_is_dir){
      char *src_base = get_basename(argv[i]);
      if(strlen(dest) + 1 + strlen(src_base) + 1 > sizeof(target)){
        fprintf(2, "cp: path too long\n");
        continue;
      }
      strcpy(target, dest);
      int len = strlen(target);
      if(len > 0 && target[len-1] != '/') {
        target[len] = '/';
        target[len+1] = '\0';
      }
      strcat(target, src_base);
      copy(argv[i], target);
    } else {
      copy(argv[i], dest);
    }
  }

  exit(0);
}
