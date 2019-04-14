#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>
#include <sys/mman.h>

using namespace std;

typedef unsigned char uchar;
typedef unsigned int uint;

typedef void* (*func_ptr)(void*);

const string SUCCESS = "[ OK ] ";
const string FAIL    = "[FAIL] ";

extern "C" {
	void* foo(void*);
};

int change_page_permissions_of_address(func_ptr);
bool compare_buffers(uchar buf1[4], uchar buf2[4]);
uchar* get_function_end(uchar*);
void decode(uchar, func_ptr);

int main(void) {
	func_ptr funcs[1] = {foo};
	vector<func_ptr> funcs_to_decode(&funcs[0], &funcs[1]);

	int key_int;
    printf("Enter key: ");
    scanf("%i", &key_int);
 	uchar key(key_int);

    for (int i = 0; i < funcs_to_decode.size(); ++i) {
	    if(change_page_permissions_of_address(funcs_to_decode[i]) == -1) {
	        fprintf(stderr, "Error while changing page permissions of foo(): %s\n", strerror(errno));
	        return 1;
	    }
	    decode(key, funcs_to_decode[i]);
    }

 	int a = 2;
 	int* arg = new int(a);
 	int b = *(int*) (*funcs_to_decode[0])(arg);
    printf("Result of foo = %i\n", b);

    return 0;
}

void* foo(void* a) {
	printf("%s entering function %s\n", SUCCESS.c_str(), __func__);
    
    int i= *(int*) a;
    i++;
    printf("i: %d\n", i);
    
    printf("%s leaving function %s\n", SUCCESS.c_str(), __func__);

    int* res = new int(i); 
    return res;
}

int change_page_permissions_of_address(func_ptr addr) {
    // Move the pointer to the page boundary
    int page_size = getpagesize();
    // addr -= (unsigned long)addr % page_size;
    char* v = (char*) addr;
    void* new_addr = v - ((unsigned long)addr % page_size);

    if(mprotect(new_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
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

void decode(uchar key, func_ptr func) {
    uchar* start = (uchar*) func;
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
