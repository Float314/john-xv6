#include "kernel/types.h"
#include "user/user.h"

void
cowsay(char *msg)
{
  int len = strlen(msg);
  printf(" ");
  for (int i = 0; i < len + 2; i++) printf("-");
  printf("\n< %s >\n ", msg);
  for (int i = 0; i < len + 2; i++) printf("-");
  printf("\n        \\   ^__^\n");
  printf("         \\  (oo)\\_______\n");
  printf("            (__)\\       )\\/\\\n");
  printf("                ||----w |\n");
  printf("                ||     ||\n");
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    cowsay("Moo!");
  } else {
    cowsay(argv[1]);
  }
  exit(0);
}
