#include "kernel/types.h"
#include "user/user.h"

#define STACK_SIZE 64

int stack[STACK_SIZE];
int top = -1;

void push(int val) {
  if (top < STACK_SIZE - 1) stack[++top] = val;
}

int pop() {
  if (top >= 0) return stack[top--];
  return 0;
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: bc expression (RPN style for simplicity, e.g., '3 4 +')\n");
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    char *s = argv[i];
    if (strcmp(s, "+") == 0) {
      push(pop() + pop());
    } else if (strcmp(s, "-") == 0) {
      int b = pop();
      int a = pop();
      push(a - b);
    } else if (strcmp(s, "*") == 0) {
      push(pop() * pop());
    } else if (strcmp(s, "/") == 0) {
      int b = pop();
      int a = pop();
      if (b != 0) push(a / b);
      else { fprintf(2, "div by zero\n"); exit(1); }
    } else {
      push(atoi(s));
    }
  }

  printf("%d\n", pop());
  exit(0);
}
