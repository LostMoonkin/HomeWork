#include "MyExt.h"

int main(int argc, const char ** argv) {

    argc--;
    argv++;
    char name[30] = "MyExtStore";
    if (argc >= 1) {
        strcpy(name, *argv);
    }

    MyExt myExt(name);
    myExt.Init();
    return 0;
}
