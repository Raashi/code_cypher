#include <iostream>

using namespace std;


void some_function() {
    cout << "Hello, World!" << endl;
}

int main() {
    void (*fp)();
    fp = &some_function;
    cout << &fp << endl;
    cout << sizeof(fp) << endl;

    int (*fmain)();
    fmain = &main;
    cout << &fmain << endl;
    cout << sizeof(fmain) << endl;

    return 0;
}