#include "kernel/types.h"
#include "user/user.h"

#define MAX_PROCS 64

struct procinfo procs[MAX_PROCS];
int num_procs;

void
print_tree(int pid, int indent)
{
  for (int i = 0; i < num_procs; i++) {
    if (procs[i].ppid == pid) {
      for (int j = 0; j < indent; j++) printf("  ");
      printf("|- %d %s (priority: %d)\n", procs[i].pid, procs[i].name, procs[i].priority);
      print_tree(procs[i].pid, indent + 1);
    }
  }
}

int
main(int argc, char *argv[])
{
  num_procs = getprocs(procs, MAX_PROCS);
  if (num_procs < 0) {
    fprintf(2, "pstree: getprocs failed\n");
    exit(1);
  }

  // Find root process (pid 1)
  for (int i = 0; i < num_procs; i++) {
    if (procs[i].pid == 1) {
      printf("%d %s (priority: %d)\n", procs[i].pid, procs[i].name, procs[i].priority);
      print_tree(procs[i].pid, 1);
      break;
    }
  }

  exit(0);
}
