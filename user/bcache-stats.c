#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct bcachestats st;
  if (bcachestats(&st) < 0) {
    fprintf(2, "bcachestats failed\n");
    exit(1);
  }

  printf("Buffer Cache Stats:\n");
  printf("  Hits:   %d\n", st.hits);
  printf("  Misses: %d\n", st.misses);
  if (st.hits + st.misses > 0) {
    printf("  Hit Rate: %d%%\n", (st.hits * 100) / (st.hits + st.misses));
  }

  exit(0);
}
