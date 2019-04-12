#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <sys/mman.h>

using namespace std;

typedef unsigned char uchar;

const string SUCCESS = "[ OK ] ";
const string FAIL    = "[FAIL] ";

extern "C" {
	void foo(void);
};

int change_page_permissions_of_address(void *addr);
bool compare_buffers(uchar buf1[4], uchar buf2[4]);
uchar* get_function_end(uchar* start);
void decode(uchar key);

int main(void) {
    void *foo_addr = (void*)foo;

    // Change the permissions of the page that contains foo() to read, write, and execute
    // This assumes that foo() is fully contained by a single page
    if(change_page_permissions_of_address(foo_addr) == -1) {
        fprintf(stderr, "Error while changing page permissions of foo(): %s\n", strerror(errno));
        return 1;
    }

    uchar key;
    unsigned int key_int;
    printf("Enter key: ");
    scanf("%i", &key_int);
 	key = key_int;

    decode(key);

    foo();

    return 0;
}

void foo(void) {
	printf("%s entering function %s\n", SUCCESS.c_str(), __func__);
    
    int i=0;
    i++;
    printf("i: %d\n", i);
    
    printf("%s leaving function %s\n", SUCCESS.c_str(), __func__);
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

bool compare_buffers(uchar* buf1[4], uchar* buf2[4]) {
	for (int i = 0; i < 4; ++i)
		if (*(buf1[i]) != *(buf2[i]))
			return false;
	return true;
}

uchar* get_function_end(uchar* start) {
	int idx = 1;
	uchar next_func_sig[4] = {0x55, 0x48, 0x89, 0xe5};

	while (true) {
		bool go_next = false;
		for (int i = 0; i < 4; ++i)
			if (*((uchar*) start + idx + i) != next_func_sig[i]) {
				go_next = true;
				break;
			}
		if (!go_next)
			break;
		idx++;
	}
	return (uchar*) start + idx;
}

void decode(uchar key) {
    void* foo_addr = (void*)foo;
    uchar* start = (uchar*) foo_addr;
    uchar* end = get_function_end(start);

    int i = 0;
    uchar* current = start;
    while (current != end) {
        *current = *current ^ key;
        current += 1;
        i++;
    }
    printf("%sdecoded\n", SUCCESS.c_str());
}
