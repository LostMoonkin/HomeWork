//
// Created by 叶梅北宁 on 11/2/16.
//

#ifndef SIMPLEFILESYSTEM_FCB_H
#define SIMPLEFILESYSTEM_FCB_H


class FCB {
public:
    unsigned int id;      //4B
    char name[30];        //30B
    unsigned char attr;   //1B
    unsigned int BlockId; //4B
};

typedef FCB * FCBPointer;

class FCBLinkNode {
public:
    FCB fcb;
    FCBLinkNode * next;
};

typedef FCBLinkNode * FCBLink;


#endif //SIMPLEFILESYSTEM_FCB_H
