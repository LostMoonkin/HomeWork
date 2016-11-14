#include "header/MyExt.h"

int main(int argc, const char ** argv) {


    argc--;
    argv++;
    char name[30] = "MyExtStore";
    if (argc >= 1) {
        strcpy(name, *argv);
    }

    MyExt myExt(name);
    myExt.Init();


    /*
     * Test for md5
     */

//    unsigned char *text = (unsigned char *) "123456";
//    MD5 iMD5;
//    iMD5.GenerateMD5(text, 6 );
//    const char *result = iMD5.ToString().c_str();
//    cout << result << endl;
//
    return 0;
}
