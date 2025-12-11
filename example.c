#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Hello World\n");
    printf("Process ID: %d\n", getpid());  // system call
    printf("User ID: %d\n", getuid());    // system call
    return 0;
}
