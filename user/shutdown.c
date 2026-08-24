/* Shutdown OS command */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() { 
    printf("Shutting down the system \n");
    shutdown(); 
    exit(0);
}