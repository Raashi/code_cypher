#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>
#include <map>
#include <sys/mman.h>

using namespace std;

typedef unsigned char uchar;
typedef unsigned int uint;

typedef void* (*func_ptr)(void*);

typedef map<string, func_ptr> mapf;

const string SUCCESS = "[ OK ] ";
const string FAIL    = "[FAIL] ";

#define ALLOW_PERMISSIONS    true
#define RESTRICT_PERMESSIONS false

extern "C" {
	void* foo(void* a) {
		printf("%s entering function %s\n", SUCCESS.c_str(), __func__);
	    
	    int i= *(int*) a;
	    i++;
	    printf("i: %d\n", i);
	    
	    printf("%s leaving function %s\n", SUCCESS.c_str(), __func__);

	    int* res = new int(i); 
	    return res;
	}

	void* bar(void* a) {
		pair<int, int> p = *(pair<int, int>*) a;

		int* res = new int(p.first);
		return res;
	}
};

void* exec_function(string, void*);
uchar get_func_key(string);

int change_page_permissions_of_address(func_ptr, bool);
bool compare_buffers(uchar buf1[4], uchar buf2[4]);
uchar* get_function_end(uchar*);
void decode(uchar, func_ptr);

mapf funcs = {
	{"foo", foo}
};

map<string, uchar> funcs_keys;

// g++ main.cpp -std=c++11 -o a
int main(void) {
 	int a = 2;
 	int b = *(int*) exec_function("foo", &a);
    printf("1. Result of foo = %i\n", b);
    b = *(int*) exec_function("foo", &b);
    printf("2. Result of foo = %i\n", b);

    return 0;
}

uchar get_func_key(string func_name) {
	map<string, uchar>::iterator it = funcs_keys.find(func_name);
	if (it != funcs_keys.end())
		return it->second;

	int key_int;
    printf("Enter key for func <%s>: ", func_name.c_str());
    scanf("%i", &key_int);
 	uchar key(key_int);
 	funcs_keys[func_name] = key;
 	return key;
}

void* exec_function(string func_name, void* arg) {
	mapf::iterator it = funcs.find(func_name);
	if (it == funcs.end()) throw runtime_error("no func '" + func_name + "' in map");

	func_ptr func = funcs[func_name];

	change_page_permissions_of_address(func, ALLOW_PERMISSIONS);

	uchar key = get_func_key(func_name);

 	decode(key, func);

 	void* res = (*func)(arg);

 	decode(key, func);

 	change_page_permissions_of_address(func, RESTRICT_PERMESSIONS);

 	return res;
}

int change_page_permissions_of_address(func_ptr addr, bool mode) {
    // Move the pointer to the page boundary
    int page_size = getpagesize();
    // addr -= (unsigned long)addr % page_size;
    char* v = (char*) addr;
    void* new_addr = v - ((unsigned long)addr % page_size);

    if (mode == ALLOW_PERMISSIONS) {
	    if(mprotect(new_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
	        return -1;
	} else {
		if(mprotect(new_addr, page_size, PROT_READ | PROT_EXEC) == -1)
	        return -1;
	}

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
    printf("%s(en/de)coded\n", SUCCESS.c_str());
}
