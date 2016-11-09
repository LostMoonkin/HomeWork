//
// Created by 叶梅北宁 on 11/2/16.
//

#ifndef SIMPLEFILESYSTEM_INODE_H
#define SIMPLEFILESYSTEM_INODE_H

#include <time.h>


class INode {
public:
    enum {
        FILE = 0, DIR,
    };
    enum {
        READ_ONLY = 0, READ_WRITE,
    };
    unsigned int id;       //4B node index
    char name[30];         //30B file name
    unsigned char attr;    //1B file attribute, 0-file 1-dir
    unsigned int parent;   //4B index of node's parent
    unsigned int length;   //4B file: file size, unsigned 2^32-1(4GB-1B), dir: files num
    unsigned char type;    //1B 0-read-only, 1-read-write
    time_t createdTime;    //8B create time of file
    time_t lastChangeTime; //8B last change time，unix time
    unsigned int addr[12]; //12*4B container index file [10] dir[11]
    unsigned int blockId;  //4B data block id
};

typedef INode * INodePointer;


#endif //SIMPLEFILESYSTEM_INODE_H
