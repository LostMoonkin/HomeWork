//
// Created by 叶梅北宁 on 11/2/16.
//

#ifndef SIMPLEFILESYSTEM_MYEXT_H
#define SIMPLEFILESYSTEM_MYEXT_H

#include <iostream>
#include <string>
#include <algorithm>
#include <sys/types.h>
#include "MD5.h"
#include "FCB.h"
#include "SuperBlock.h"
#include "INode.h"
#include "User.h"

using namespace std;


#define BLOCK_SIZE 1024
#define BLOCK_NUM 102400
#define DIRECT_INDEX 10
#define SECOND_INDEX_POSITION 10
#define THIRD_INDEX_POSITION 11
#define MYEXT_FILENAME "MyExtStore"


enum {
    SUDO = 0, HELP, LS, CD, MKDIR,
    TOUCH, CAT, WRITE, RM, MV,
    CP, CHMOD, LOGOUT, EXIT, SYSINFO,
    CLEAR, PASSWD,
    LAST,
};

enum {
    OVER_WRITE = 0, APPEND,
};


class MyExt {

public:

    //init
    MyExt(const char * fileName);
    virtual ~MyExt();
    int Init();

    //command
    void Help();
    void Ls();
    void Ls_l();
    int Cd(const char * dirName);

    //file operation
    int CreateFile(const char * fileName, unsigned char attr);
    int Read(const char * fileName);
    int Write(const char * fileName, unsigned int method);
    int Rm(const char * fileName);
    int Mv(const char * fileName, const char* newFileName);
    int Cp(const char * src, char* dst);
    int Chmod(const char * Filename, unsigned char type);

    //system
    void LogIn();
    void LogOut();
    void Exit();
    void SystemInfo();
    void Clr();
    int Passwd();

    void Command(void);


protected:

private:

    const int blockNum;
    const int blockSize;
    const char * salty = "e.t/";
    unsigned char * blockBitMap;
    unsigned char * iNodeBitMap;

    unsigned short userSize;
    unsigned short superBlockSize;
    unsigned int blockBitMapSize;
    unsigned int iNodeBitMapSize;
    unsigned short iNodeSize;
    unsigned short fcbSize;
    unsigned short itemSize;

    //offset
    unsigned long sOffset;     //superBlock
    unsigned long bbOffset;    //blockBitMap
    unsigned long ibOffset;    //iNodeBitMap
    unsigned long iOffset;     //iNode
    unsigned long bOffset;     //block

    unsigned char isAlive;

    char myExtFileName[30];
    FILE * myExtFilePointer;
    SuperBlock superBlock;
    string currentPath;
    INode currentINode;
    FCBLink currentFCBLink;

    User user;
    string userCommand[5];
    string systemCommand[LAST];

    //init
    void CreateMyExt();
    void OpenMyExt();

    void GetUser(UserPointer userPointer);
    void SetUser(User user);

    void GetSuperBlock(SuperBlockPointer superBlockPointer);
    void UpdateSuperBlock(SuperBlock superBlock);

    //blockBitMap
    unsigned int GetAvailableBlockId();
    void GetBlockBitMap(unsigned char * bitMap);
    void UpdateBlockBitMap(unsigned char * bitMap, unsigned int index);
    void UpdateBlockBitMap(unsigned char * bitMap, unsigned int start, unsigned int count);

    //iNodeBitMap
    unsigned int GetAvailableiNodeId();
    void GetINodeBitMap(unsigned char * bitmap);
    void UpdateINodeBitMap(unsigned char * bitmap, unsigned int index);
    void UpdateINodeBitMap(unsigned char * bitmap, unsigned int start, unsigned int count);

    //iNode block区域操作
    void GetINode(INodePointer iNodePointer, unsigned int id);
    void UpdateINode(INode iNode);
    void ReleaseINode(unsigned int id);

    //data block
    unsigned int GetAvailableFileItem(INode& iNode, unsigned int * availableIndex);
    unsigned int GetItem(unsigned int blockId, unsigned int index);
    void UpdateItem(unsigned int blockId, unsigned int index, unsigned int value);
    void ReleaseItem(unsigned int blockId, unsigned int id);

    //file operate
    int GetData(unsigned int blockId, char * buff, unsigned int size, unsigned int offset);
    int WriteData(unsigned int blockId, char * buff, unsigned int size, unsigned int offset);
    void ReleaseBlock(unsigned int blockId);
    
    //second, third index
    int GetSecondIndex(unsigned int blockId, unsigned int * secondIndex, unsigned int size);

    //locate
    unsigned int FindChildINode(FCBLink curLink, const char* name);
    int BackToParent();
    int SetToChild(const char * dirName);
    void BackToRoot();
    void Del(const char * fileName);

    //file write
    void OverWrite(INodePointer iNodePointer);
    void AppendWrite(INodePointer iNodePointer);

    //目录信息链操作
    void GetFCBLinkNode(FCBLink fcbLink, INode iNode);
    void GetFCBLink(FCBLink & curLink, INode iNode);
    void AppendFCBLinkNode(FCBLink curLink, INode iNode);
    void RemoveFCBLinkNode(FCBLink curLink, INode iNode);
    void RemoveFCBLinkNode(FCBLink curLink, const char * name);
    void ReleaseFCBLink(FCBLink & curLink);
    

    //system command
    int Analyse(const char * str);
    void StopHandle(int sig);
    void UpdateResource();
    void ShowPath();

    //util
    void ShowFileDigest(FCBLink pNode);
    void ShowFileDetail(INodePointer iNodePointer);
    unsigned int WaitForInput(char * buff, unsigned int limit);

};


#endif //SIMPLEFILESYSTEM_MYEXT_H
