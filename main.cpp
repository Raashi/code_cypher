#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

using namespace std;

typedef unsigned char uchar;

extern "C" {
	void foo(void);
};

int change_page_permissions_of_address(void *addr);
void decode();

int main(void) {
    void *foo_addr = (void*)foo;

    // Change the permissions of the page that contains foo() to read, write, and execute
    // This assumes that foo() is fully contained by a single page
    if(change_page_permissions_of_address(foo_addr) == -1) {
        fprintf(stderr, "Error while changing page permissions of foo(): %s\n", strerror(errno));
        return 1;
    }

    decode();

    puts("Calling foo...");
    foo();

    return 0;
}

void foo(void) {
	puts(__func__);
    int i=0;
    i++;
    printf("i: %d\n", i);
}

int change_page_permissions_of_address(void *addr) {
    // Move the pointer to the page boundary
    int page_size = getpagesize();
    // addr -= (unsigned long)addr % page_size;
    addr = static_cast<char *>(addr) - ((unsigned long)addr % page_size);

    if(mprotect(addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
        return -1;

    return 0;
}

void decode() {
    const uchar key = 56;

    void *foo_addr = (void*)foo;
    uchar *start = (uchar*) foo_addr;

    uchar buffer[3] = {0, 0, 0};

    int count = 0;

    while ((count < 3) || not ((buffer[0] == 0xc3) && (buffer[1] == 0x66) && (buffer[2] == 0x66))) {
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];

        uchar *instruction = (uchar*) foo_addr + count;
        *instruction = *instruction ^ key;

        buffer[2] = *instruction;
        count++;
    }
}
