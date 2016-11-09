//
// Created by 叶梅北宁 on 11/2/16.
//

#ifndef SIMPLEFILESYSTEM_SUPERBLOCK_H
#define SIMPLEFILESYSTEM_SUPERBLOCK_H

class SuperBlock {
public:
    unsigned short blockSize; //2B
    unsigned int blockNum; //4B
    unsigned int iNodeNum; //4B
    unsigned int blockFree; //4B
};

typedef SuperBlock * SuperBlockPointer;

#endif //SIMPLEFILESYSTEM_SUPERBLOCK_H
