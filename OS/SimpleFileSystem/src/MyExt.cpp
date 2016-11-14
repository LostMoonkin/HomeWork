//
// Created by 叶梅北宁 on 11/2/16.
//

#include "header/MyExt.h"

MyExt::MyExt(const char * fileName) : blockNum(BLOCK_NUM), blockSize(BLOCK_SIZE) {

    blockBitMap = new unsigned char[BLOCK_NUM + 1];
    iNodeBitMap = new unsigned char[BLOCK_NUM + 1];
    isAlive = 0;
    myExtFilePointer = nullptr;

    userSize = sizeof(user);
    superBlockSize = sizeof(superBlock);
    blockBitMapSize = (unsigned int) blockNum;
    iNodeBitMapSize = (unsigned int) blockNum;
    iNodeSize = sizeof(INode);
    fcbSize = sizeof(FCB);
    itemSize = sizeof(unsigned int);

    sOffset = userSize;
    bbOffset = sOffset + superBlockSize;
    ibOffset = bbOffset + blockBitMapSize;
    iOffset = ibOffset + iNodeBitMapSize;
    bOffset = iOffset + iNodeSize * blockNum;

    if (fileName == nullptr || !strcmp(fileName, "")) {
        strcpy(myExtFileName, MYEXT_FILENAME);
    } else {
        strcpy(myExtFileName, fileName);
    }

    char cmd[] = "sudo help ls cd mkdir touch cat write rm mv cp chmod logout exit sysinfo clear passwd";
    char * p;
    p = strtok(cmd, " ");
    for (int i = 0; p; p = strtok(NULL, " "), i++) {
        systemCommand[i] = p;
    }

}

MyExt::~MyExt() {

    if (!blockBitMap) {
        delete blockBitMap;
        blockBitMap = nullptr;
    }
    if (!iNodeBitMap) {
        delete iNodeBitMap;
        iNodeBitMap = nullptr;
    }
    if (currentFCBLink) {
        delete currentFCBLink;
        currentFCBLink = nullptr;
    }
}

int MyExt::Init() {
    if (isAlive) {
        exit(-1);
    }

    OpenMyExt();
    LogIn();
    Command();

    return 0;
}

void MyExt::Command(void) {
    char input[128];
    while (1) {
        ShowPath();
        fgets(input, 128, stdin);
        switch(Analyse(input)) {
            case HELP:
                Help();
                break;
                
            case LS://执行ls或ls-l
                if(userCommand[1] == "-l" || userCommand[1] == "-L") {
                    Ls_l();
                } else {
                    Ls();
                }
                break;

            case MKDIR:
                CreateFile(userCommand[1].c_str(),1);
                break;
                
            case TOUCH:
                CreateFile(userCommand[1].c_str(),0);
                break;

            case CD:
                Cd(userCommand[1].c_str());
                break;

            case CAT:
                Read(userCommand[1].c_str());
                break;
                
            case WRITE:
                if (userCommand[1] == "-o" || userCommand[1] == "-O") {
                    Write(userCommand[2].c_str(), OVER_WRITE);
                } else if (userCommand[1] == "-A" || userCommand[1] == "-a") {
                    Write(userCommand[2].c_str(), APPEND);
                } else {
                    printf("Use write -o {file} or wirte -a {file} to write!\n");
                }
                break;
                
            case LOGOUT:
                LogOut();
                break;
                
            case EXIT:
                Exit();
                return;
                break;
                
            case RM:
                if(userCommand[1] == "-r") {
                    Del(userCommand[2].c_str());
                }
                else {
                    Del(userCommand[1].c_str());
                }
                break;
                
            case SYSINFO:
                SystemInfo();
                break;
                
            case CLEAR:
                Clr();
                break;
//            case PASSWD:
//                Passwd();
//                break;
            default:
                printf("unsupported command\n");
                break;
        }
    }
}

void MyExt::CreateMyExt() {

    printf("Creating MyExt File:\n");

    if (!(myExtFilePointer = fopen(myExtFileName, "wb+"))) {
        printf("open file %s error...\n", myExtFileName);
        exit(-1);
    }

    //init user
    printf("Init User:\n");
    printf("Input UserName(0 < len < 10):");
    fgets(user.userName, sizeof(user.userName), stdin);
    if(user.userName[strlen(user.userName) - 1] == '\n') {
        user.userName[strlen(user.userName) - 1] = '\0';
    }
    system("stty -echo");
    printf("Input PassWord(0 < len < 16):");
    char password[16];
    fgets(password, sizeof(password), stdin);
    if(password[strlen(password) - 1] == '\n') {
        password[strlen(password) - 1] = '\0';
    }

    unsigned char * src = (unsigned char *)malloc(strlen(password) + strlen(salty) + 1);
    strcpy((char *) src, password);
    strcat((char *) src, salty);
    MD5 iMD5;
    iMD5.GenerateMD5(src, strlen((char *)src));
    memset(user.passWord, 0, strlen(user.passWord));
    strcpy(user.passWord, iMD5.ToString().c_str());

    system("stty echo");
    printf("\n");
    printf("Init User Success!\nusername:%s\npassword:%s\n", user.userName, password);
    SetUser(user);

    printf("\nInit Super Block...\n");
    //init super block
    superBlock.blockNum = (unsigned int) blockNum;
    superBlock.blockSize = (unsigned short) blockSize;
    superBlock.iNodeNum = 1;
    superBlock.blockFree = (unsigned int) (blockNum - 1);
    UpdateSuperBlock(superBlock);

    printf("\nInit BitMap...\n");
    //init two bitmap
    blockBitMap[0] = iNodeBitMap[0] = 1;

    for (int i = 1; i < blockNum; i++) {
        blockBitMap[i] = iNodeBitMap[i] = 0;
    }

    UpdateBlockBitMap(blockBitMap, 0, blockBitMapSize);
    UpdateINodeBitMap(iNodeBitMap, 0, iNodeBitMapSize);

    int len = (iNodeSize + blockSize) * blockNum;
    for (int i = 0; i < len; i++) {
        fputc(0, myExtFilePointer);
    }

    printf("\nInit root directory...\n");
    //init root dir
    currentINode.id = 0;
    strcpy(currentINode.name, "/");
    currentINode.attr = currentINode.DIR;
    currentINode.parent = iNodeSize;
    currentINode.length = 1;
    currentINode.type = currentINode.READ_ONLY;
    time(&(currentINode.createdTime));
    time(&(currentINode.lastChangeTime));
    memset(currentINode.addr, 0, sizeof(currentINode.addr));
    currentINode.blockId = 0;

    //write node
    UpdateINode(currentINode);
    fflush(myExtFilePointer);

    GetFCBLink(currentFCBLink, currentINode);
    currentPath = "/";
    printf("create file system %s finish.\n", this->myExtFileName);
}

void MyExt::OpenMyExt() {

    if (!(myExtFilePointer = fopen(myExtFileName, "rb"))) {
        CreateMyExt();
    } else {
        printf("open file system...\n");
        if (!(myExtFilePointer = fopen(myExtFileName, "rb+"))) {
            printf("open file %s error...\n", myExtFileName);
            exit(-1);
        }

        rewind(myExtFilePointer);
        //read header
        GetUser(&user);
        GetSuperBlock(&superBlock);
        GetBlockBitMap(blockBitMap);
        GetINodeBitMap(iNodeBitMap);

        //get current iNode
        GetINode(&currentINode, 0);
        GetFCBLink(currentFCBLink, currentINode);

        currentPath = "/";
        printf("open file system %s success\n", this->myExtFileName);
    }
}

void MyExt::GetUser(UserPointer userPointer) {

    if (!myExtFilePointer || !userPointer) {
        return;
    }

    rewind(myExtFilePointer);
    fread(userPointer, userSize, 1, myExtFilePointer);
}

void MyExt::SetUser(User user) {

    if (!myExtFilePointer) {
        return;
    }

    rewind(myExtFilePointer);
    fwrite(&user, userSize, 1, myExtFilePointer);
}

void MyExt::GetSuperBlock(SuperBlockPointer superBlockPointer) {

    if (!myExtFilePointer || !superBlockPointer) {
        return;
    }

    fseek(myExtFilePointer, sOffset, SEEK_SET);
    fread(superBlockPointer, superBlockSize, 1, myExtFilePointer);
}

void MyExt::UpdateSuperBlock(SuperBlock superBlock) {

    if (!myExtFilePointer) {
        return;
    }

    fseek(myExtFilePointer, sOffset, SEEK_SET);
    fwrite(&superBlock, superBlockSize, 1, myExtFilePointer);
}

unsigned int MyExt::GetAvailableBlockId() {

    if (superBlock.blockFree <= 0) {
        printf("no free block\n");
        return 0;
    }

    for (unsigned int i = 0; i < blockNum; i++) {
        if (!blockBitMap[i]) {
            ReleaseBlock(i);
            return i;
        }
    }

    printf("no free block\n");
    return 0;
}

void MyExt::GetBlockBitMap(unsigned char *bitMap) {

    if (!myExtFilePointer || !bitMap) {
        return;
    }

    fseek(myExtFilePointer, bbOffset, SEEK_SET);
    fread(bitMap, blockBitMapSize, 1, myExtFilePointer);
}

void MyExt::UpdateBlockBitMap(unsigned char *bitMap, unsigned int index) {

    UpdateBlockBitMap(bitMap, index, 1);

}

void MyExt::UpdateBlockBitMap(unsigned char *bitMap, unsigned int start, unsigned int count) {

    if (!myExtFilePointer) {
        return;
    }

    fseek(myExtFilePointer, bbOffset + start, SEEK_SET);
    fwrite(bitMap + start, count, 1, myExtFilePointer);
}

unsigned int MyExt::GetAvailableiNodeId() {

    for (unsigned int i = 0; i < blockNum; i++) {
        if (!iNodeBitMap[i]) {
            return i;
        }
    }
    return 0;
}

void MyExt::GetINodeBitMap(unsigned char *bitmap) {

    if (!myExtFilePointer || !bitmap) {
        return;
    }

    fseek(myExtFilePointer, ibOffset, SEEK_SET);
    fread(bitmap, iNodeBitMapSize, 1, myExtFilePointer);
}

void MyExt::UpdateINodeBitMap(unsigned char *bitmap, unsigned int index) {

    UpdateBlockBitMap(bitmap, index, 1);

}

void MyExt::UpdateINodeBitMap(unsigned char *bitmap, unsigned int start, unsigned int count) {

    if (!myExtFilePointer) {
        return;
    }

    fseek(myExtFilePointer, ibOffset + start, SEEK_SET);
    fwrite(bitmap + start, count, 1, myExtFilePointer);
}

void MyExt::GetINode(INodePointer iNodePointer, unsigned int id) {

    if (!myExtFilePointer || !iNodePointer) {
        return;
    }

    fseek(myExtFilePointer, iOffset + iNodeSize * id, SEEK_SET);
    fread(iNodePointer, iNodeSize, 1, myExtFilePointer);
}

void MyExt::UpdateINode(INode iNode) {

    if (!myExtFilePointer) {
        return;
    }

    fseek(myExtFilePointer, iOffset + iNodeSize * iNode.id, SEEK_SET);
    fwrite(&iNode, iNodeSize, 1, myExtFilePointer);
}

void MyExt::ReleaseINode(unsigned int id) {

    if (!myExtFilePointer) {
        return;
    }

    fseek(myExtFilePointer, iOffset + iNodeSize * id, SEEK_SET);
    for (int i = 0; i < iNodeSize; i++) {
        fputc(0, myExtFilePointer);
    }
}

unsigned int MyExt::GetAvailableFileItem(INode &iNode, unsigned int *availableIndex) {

    unsigned int index, blockId, fileItem;

    const int itemCount = blockSize / itemSize;

    for (int i = 0; i < DIRECT_INDEX; i++) {

        blockId = iNode.addr[i];

        if (blockId > 0 || iNode.id == 0) {

            for (index = 0; index < itemCount; index++) {

                fileItem = GetItem(blockId, index);

                if (!fileItem) {
                    *availableIndex = index;
                    return blockId;
                }
            }
        } else {

            blockId = GetAvailableBlockId();

            if (blockId > 0) {

                superBlock.blockFree --;
                UpdateSuperBlock(superBlock);

                blockBitMap[blockId] = 1;
                UpdateBlockBitMap(blockBitMap, blockId);

                iNode.addr[i] = blockId;
                UpdateINode(iNode);

                *availableIndex = 0;
                return blockId;
            } else {
                return 0;
            }
        }
    }

//    second index
    unsigned int * secondIndex = new unsigned int[itemCount + 1];
    unsigned int addrBlockId = iNode.addr[SECOND_INDEX_POSITION];
    
    if (addrBlockId) {
        
        GetSecondIndex(addrBlockId, secondIndex, blockSize);
        
        for (int i = 0; i < itemCount; i++) {
            
            blockId = secondIndex[i];
            
            if (blockId > 0 || iNode.id == 0) {
                
                for (index = 0; index < itemCount; index++) {
                    
                    fileItem = GetItem(blockId, index);
                    
                    if (!fileItem) {
                        *availableIndex = index;
                        return blockId;
                    }
                }
            } else {
                
                blockId = GetAvailableBlockId();
                
                if (blockId > 0) {
                    
                    superBlock.blockFree --;
                    UpdateSuperBlock(superBlock);
                    
                    blockBitMap[blockId] = 1;
                    UpdateBlockBitMap(blockBitMap, blockId);
                    
                    iNode.addr[i] = blockId;
                    UpdateINode(iNode);
                    
                    *availableIndex = 0;
                    delete [] secondIndex;
                    return blockId;
                } else {
                    delete [] secondIndex;
                    return 0;
                }
            }
        }
    } else {
        
        addrBlockId = GetAvailableBlockId();
        
        if (addrBlockId) {
            
            superBlock.blockFree --;
            UpdateSuperBlock(superBlock);
            
            blockBitMap[addrBlockId] = 1;
            UpdateBlockBitMap(blockBitMap, addrBlockId);
            
            iNode.addr[SECOND_INDEX_POSITION] = addrBlockId;
            UpdateINode(iNode);
            
            GetSecondIndex(addrBlockId, secondIndex, blockSize);
            
            for (int i = 0; i < itemCount; i++) {
                
                blockId = secondIndex[i];
                
                if (blockId > 0 || iNode.id == 0) {
                    
                    for (index = 0; index < itemCount; index++) {
                        
                        fileItem = GetItem(blockId, index);
                        
                        if (!fileItem) {
                            *availableIndex = index;
                            return blockId;
                        }
                    }
                } else {
                    
                    blockId = GetAvailableBlockId();
                    
                    if (blockId > 0) {
                        
                        superBlock.blockFree --;
                        UpdateSuperBlock(superBlock);
                        
                        blockBitMap[blockId] = 1;
                        UpdateBlockBitMap(blockBitMap, blockId);
                        
                        iNode.addr[i] = blockId;
                        UpdateINode(iNode);
                        
                        *availableIndex = 0;
                        delete [] secondIndex;
                        return blockId;
                    } else {
                        delete [] secondIndex;
                        return 0;
                    }
                }
            }
            
        } else {
            delete[] secondIndex;
            return 0;
        }
    }
    delete [] secondIndex;
    return 0;
}

unsigned int MyExt::GetItem(unsigned int blockId, unsigned int index) {

    unsigned int value = 0;

    if (!myExtFilePointer) {
        return value;
    }

    fseek(myExtFilePointer, bOffset + blockSize * blockId + itemSize * index, SEEK_SET);
    fread(&value, itemSize, 1, myExtFilePointer);

    return value;
}

void MyExt::UpdateItem(unsigned int blockId, unsigned int index, unsigned int value) {

    if (!myExtFilePointer) {
        return;
    }

    fseek(myExtFilePointer,  bOffset + blockSize * blockId + itemSize * index, SEEK_SET);
    fwrite(&value, itemSize, 1, myExtFilePointer);
}

void MyExt::ReleaseItem(unsigned int blockId, unsigned int id) {

    const int itemCount = blockSize / itemSize;
    unsigned int itemId;
    fseek(myExtFilePointer, bOffset + blockSize * blockId, SEEK_SET);

    for (int i = 0; i < itemCount; i++) {

        fread(&itemId, itemSize, 1, myExtFilePointer);

        if (itemId == id) {
            fseek(myExtFilePointer, -itemSize, SEEK_CUR);

            itemId = 0;

            fwrite(&itemId, itemSize, 1, myExtFilePointer);
        }
    }
}

int MyExt::GetData(unsigned int blockId, char *buff, unsigned int size, unsigned int offset) {

    int len = 0;

    if (!myExtFilePointer || !buff || offset > blockSize) {
        return len;
    }

    fseek(myExtFilePointer, bOffset + blockSize * blockId, SEEK_SET);

    if (size > blockSize - offset) {
        size = blockSize - offset;
    }

    len = (int)fread(buff, 1, size, myExtFilePointer);

    return len;
}

int MyExt::GetSecondIndex(unsigned int blockId, unsigned int * secondIndex, unsigned int size) {
    
    int len = 0;
    
    if (!myExtFilePointer || !secondIndex) {
        return len;
    }
    
    fseek(myExtFilePointer, bOffset + blockSize * blockId, SEEK_SET);
    
    len = (int) fread(secondIndex, size, 1, myExtFilePointer);
    
    return len;
    
}

int MyExt::WriteData(unsigned int blockId, char *buff, unsigned int size, unsigned int offset) {

    int len = 0;

    if (!myExtFilePointer || !buff || offset > blockSize) {
        return len;
    }

    fseek(myExtFilePointer, bOffset + blockSize * blockId + offset, SEEK_SET);

    if(size > blockSize-offset) {
        size = blockSize - offset;
    }

    len = (int) fwrite(buff, size, 1, myExtFilePointer);

    return len;
}

void MyExt::ReleaseBlock(unsigned int blockId) {

    if (!myExtFilePointer) {
        return;
    }

    fseek(myExtFilePointer, bOffset + blockSize * blockId, SEEK_SET);
    for (int i = 0; i < blockSize; i++) {
        fputc(0, myExtFilePointer);
    }

}

unsigned int MyExt::FindChildINode(FCBLink curLink, const char *name) {
    
    if (!curLink || !name) {
        return 0;
    }
    
    FCBLink link = curLink->next;
    
    while (link) {
        if (!strcmp(link->fcb.name, name)) {
            return link->fcb.id;
        }
        link = link->next;
    }
    
    return 0;
}

void MyExt::GetFCBLinkNode(FCBLink fcbLink, INode iNode) {

    if (!fcbLink) {
        return;
    }

    fcbLink->fcb.id = iNode.id;
    strcpy(fcbLink->fcb.name, iNode.name);
    fcbLink->fcb.attr = iNode.attr;
    fcbLink->fcb.BlockId = iNode.blockId;
    fcbLink->next = NULL;

}

void MyExt::GetFCBLink(FCBLink &curLink, INode iNode) {

    if (!curLink) {
        ReleaseFCBLink(curLink);
    }

    curLink = new FCBLinkNode();
    GetFCBLinkNode(curLink, iNode);

    unsigned int index, blockId, fileItem;
    INode fileINode;
    FCBLink fcbLink, link = curLink;
    unsigned long len = iNode.length;
    const int itemCount = blockSize / itemSize;
    
    GetINode(&fileINode, iNode.id);
    memset(fileINode.name, 0, sizeof(fileINode.name));
    fileINode.name[0] = '.';
    fcbLink = new FCBLinkNode();
    GetFCBLinkNode(fcbLink, fileINode);
    link->next = fcbLink;
    link = fcbLink;
    len --;
    
    if (iNode.id) {
        GetINode(&fileINode, iNode.parent);
        memset(fileINode.name, 0, sizeof(fileINode.name));
        fileINode.name[0] = '.';
        fileINode.name[1] = '.';
        fcbLink = new FCBLinkNode();
        GetFCBLinkNode(fcbLink, fileINode);
        link->next = fcbLink;
        link = fcbLink;
        len --;
    }
    
    if (iNode.length <= 0) {
        return;
    }
    

    for (int i = 0; i < DIRECT_INDEX; i++) {
        blockId = iNode.addr[i];

        if (blockId > 0 || currentINode.id == 0) {

            for (index = 0; index < itemCount; index++) {
                fileItem = GetItem(blockId, index);

                if (fileItem > 0) {
                    GetINode(&fileINode, fileItem);
                    fcbLink = new FCBLinkNode();

                    GetFCBLinkNode(fcbLink, fileINode);
                    link->next = fcbLink;
                    link = fcbLink;
                    len --;

                    if (len <= 0) {
                        return;
                    }
                }
            }
        }
    }


    //second index
    unsigned int * secondIndex = new unsigned int [itemCount + 1];
    unsigned int addrBlockId = iNode.addr[SECOND_INDEX_POSITION];

    if (addrBlockId) {
        
        GetSecondIndex(addrBlockId, secondIndex, blockSize);
        
        for (int i = 0; i < itemCount; i++) {
            blockId = secondIndex[i];

            if (blockId > 0) {

                for (index = 0; index < itemCount; index++) {
                    fileItem = GetItem(blockId, index);

                    if (fileItem > 0) {
                        GetINode(&fileINode, fileItem);
                        fcbLink = new FCBLinkNode();

                        GetFCBLinkNode(fcbLink, fileINode);
                        link->next = fcbLink;
                        link = fcbLink;
                        len --;

                        if (len <= 0) {
                            delete [] secondIndex;
                            return;
                        }
                    }
                }
            }
        }
    }
    
    delete [] secondIndex;

}

void MyExt::AppendFCBLinkNode(FCBLink curLink, INode iNode) {

    if (!curLink || iNode.id <= 0) {
        return;
    }

    FCBLink link = curLink;
    while (link->next) {
        link = link->next;
    }

    FCBLink fcbLink = new FCBLinkNode();

    GetFCBLinkNode(fcbLink, iNode);
    link->next = fcbLink;
}

void MyExt::RemoveFCBLinkNode(FCBLink curLink, INode iNode) {

    if (!curLink || iNode.length <= 0) {
        return;
    }

    FCBLink link = curLink->next;
    FCBLink last = curLink;

    while (link) {
        if (link->fcb.id == iNode.id) {
            last->next = link->next;
            delete link;
            break;
        }
        last = link;
        link = link->next;
    }
}

void MyExt::RemoveFCBLinkNode(FCBLink curLink, const char *name) {

    if (!curLink || !name) {
        return;
    }

    FCBLink link = curLink->next;
    FCBLink last = curLink;

    while (link) {
        if (!strcmp(link->fcb.name, name)) {
            last->next = link->next;
            delete link;
            break;
        }

        last = link;
        link = link->next;
    }
}

void MyExt::ReleaseFCBLink(FCBLink &curLink) {

    FCBLink link = curLink;
    FCBLink tempLink;

    while (link) {
        tempLink = link->next;
        delete link;
        link = tempLink;
    }

    curLink = NULL;
}

void MyExt::Help() {

    printf(
            "command: \n"
                    "help    ---  show help menu \n"
                    "sysinfo ---  show system base information \n"
                    "logout  ---  exit user \n"
                    "account ---  modify username and password \n"
                    "clear   ---  clear the screen \n"
                    "ls      ---  list the digest of the directory's children \n"
                    "ls -l   ---  list the detail of the directory's children \n"
                    "cd      ---  change directory \n"
                    "mkdir   ---  make directory   \n"
                    "touch   ---  create a new file \n"
                    "cat     ---  read a file \n"
                    "write   ---  write something to a file \n"
                    "rm      ---  delete a directory or a file \n"
                    "cp      ---  cp a directory file to another directory or file (not finish)\n"
                    "mv      ---  rename a file or directory \n"
                    "chmod   ---  change the authorizatino of a directory or a file \n"
                    "exit    ---  exit this system\n"
    );
}

void MyExt::Ls() {

    FCBLink link = currentFCBLink->next;

    while (link) {
        ShowFileDigest(link);

        link = link->next;
    }

}

void MyExt::Ls_l() {

    FCBLink link = currentFCBLink->next;
    INodePointer iNodePointer = new INode();

    while (link) {
        iNodePointer->id = link->fcb.id;
        GetINode(iNodePointer, iNodePointer->id);
        if (iNodePointer->id == currentINode.id) {
            iNodePointer->name[0] = '.';
            iNodePointer->name[1] = 0;
        } else if (currentINode.id != 0 && iNodePointer->id == currentINode.parent) {
            iNodePointer->name[0] = '.';
            iNodePointer->name[1] = '.';
            iNodePointer->name[2] = 0;
        }
        ShowFileDetail(iNodePointer);

        link = link->next;
    }
}

int MyExt::BackToParent() {

    unsigned int id = currentINode.parent;

    if (currentINode.id > 0) {
        GetINode(&currentINode, id);
        GetFCBLink(currentFCBLink, currentINode);

        unsigned long pos = currentPath.rfind('/', currentPath.length() - 2);
        currentPath.erase(pos + 1, currentPath.length() - 1 - pos);

        return 0;
    }

    return -1;
}

void MyExt::BackToRoot() {

    unsigned int id = 0;

    GetINode(&currentINode, id);
    GetFCBLink(currentFCBLink, currentINode);

    currentPath = "/";
}

int MyExt::SetToChild(const char *dirName) {

    unsigned int id = FindChildINode(currentFCBLink, dirName);
    
    if (!strncmp(dirName, "..", 2)) {
        return BackToParent();
    }
    if (!strncmp(dirName, ".", 1)) {
        return 0;
    }

    if (id > 0) {

        INode tempINode;

        GetINode(&tempINode, id);
        if (tempINode.attr == tempINode.FILE) {

            printf("%s is not a directory!\n", tempINode.name);

            return -1;
        } else {

            GetINode(&currentINode, id);
            GetFCBLink(currentFCBLink, currentINode);
            currentPath.append(dirName);
            currentPath.append("/");

            return 0;
        }
    } else {

        printf("No such directory: %s!\n", dirName);

        return -1;
    }
}

int MyExt::Cd(const char *dirName) {
    string tempPath = currentPath;
    char * tempDirName;
    tempDirName = (char *) calloc(strlen(dirName), sizeof(char));
    strcpy(tempDirName, dirName);
    
    char * p = strtok(tempDirName, "/");
    unsigned int flag = 0;
    
    for (; p; p = strtok(NULL, "/")) {
    
        if (SetToChild(p)) {
            flag = 1;
            break;
        }
    }
    
    if (flag) {
        Cd(tempPath.c_str());
        return -1;
    }
    
    return 0;
}

int MyExt::Mv(const char * fileName, const char * newFileName) {
    return 0;
    
}

/*
 * Old Cd
 */
//int MyExt::Cd(const char *dirName) {
//
//    string tempPath = currentPath;
//    char * tempDirName;
//    tempDirName = (char *) calloc(strlen(dirName), sizeof(char));
//    strcpy(tempDirName, dirName);
//
//    if (!strcmp(dirName, ".")) {
//        return 0;
//    }
//
//    if (!strcmp(dirName, "..")) {
//
//        if (BackToParent()) {
//            return 0;
//        } else {
//            return -1;
//        }
//    }
//
//    if (!strncmp(dirName, "./", 2) || (dirName[0] != '.' && dirName[0] != '/')) {
//
//        if (!strncmp(dirName, "./", 2)) {
//            tempDirName += 2;
//        }
//
//        char * p = strtok(tempDirName, "/");
//        unsigned int flag = 0;
//
//        for (; p; p = strtok(NULL, "/")) {
//
//            if (!strcmp(p, ".")) {
//                continue;
//            }
//
//            if (!strcmp(p, "..")) {
//                if (BackToParent()) {
//                    flag = 1;
//                    break;
//                } else {
//                    continue;
//                }
//            }
//
//            if (SetToChild(p)) {
//                flag = 1;
//                break;
//            }
//        }
//
//        if (flag) {
//            Cd(tempPath.c_str());
//            return -1;
//        }
//
//        return 0;
//    }
//
//    if (!strncmp(dirName, "../", 3)) {
//
//        tempDirName += 3;
//
//        if (BackToParent()) {
//            Cd(tempPath.c_str());
//        }
//
//        char * p = strtok(tempDirName, "/");
//        unsigned int flag = 0;
//
//        for (; p; p = strtok(NULL, "/")) {
//
//            if (!strcmp(p, ".")) {
//                continue;
//            }
//
//            if (!strcmp(p, "..")) {
//                if (BackToParent()) {
//                    flag = 1;
//                    break;
//                } else {
//                    continue;
//                }
//            }
//
//            if (SetToChild(p)) {
//                flag = 1;
//                break;
//            }
//        }
//
//        if (flag) {
//            Cd(tempPath.c_str());
//            return -1;
//        }
//
//        return 0;
//    }
//
//    if (!strncmp(dirName, "/", 1)) {
//
//        tempDirName += 1;
//
//        BackToRoot();
//
//        char * p = strtok(tempDirName, "/");
//        unsigned int flag = 0;
//
//        for (; p; p = strtok(NULL, "/")) {
//
//            if (!strcmp(p, ".")) {
//                continue;
//            }
//
//            if (!strcmp(p, "..")) {
//                if (BackToParent()) {
//                    flag = 1;
//                    break;
//                } else {
//                    continue;
//                }
//            }
//
//            if (SetToChild(p)) {
//                flag = 1;
//                break;
//            }
//        }
//
//        if (flag) {
//            BackToRoot();
//            printf("Unexpected error, back to root!\n");
//            return -1;
//        }
//
//        return 0;
//
//    }
//
//    return 0;
//}

int MyExt::CreateFile(const char *fileName, unsigned char attr) {

    if (!fileName || !strcmp(fileName, "") || FindChildINode(currentFCBLink, fileName) > 0) {
        printf("invalid file name:the name is empty,or the file has existed\n");

        return -1;
    }

    unsigned int index, dirBlockId = GetAvailableFileItem(currentINode, &index);

    if (dirBlockId || !currentINode.id) {
        unsigned int blockId = GetAvailableBlockId();

        if (blockId) {

            superBlock.blockFree --;
            superBlock.iNodeNum ++;
            blockBitMap[blockId] = 1;
            UpdateBlockBitMap(blockBitMap, blockId);

            unsigned int id = GetAvailableiNodeId();
            INodePointer iNodePointer = new INode();

            iNodePointer->id = id;
            strcpy(iNodePointer->name, fileName);
            iNodePointer->attr = attr;
            if (attr == iNodePointer->DIR) {
                
            }
            iNodePointer->parent = currentINode.id;
            iNodePointer->length = iNodePointer->attr == iNodePointer->DIR ? 2 : 0;
            iNodePointer->type = iNodePointer->READ_WRITE;
            time(&iNodePointer->createdTime);
            time(&iNodePointer->lastChangeTime);

            iNodePointer->blockId = blockId;

            UpdateINode(*iNodePointer);

            iNodeBitMap[id] = 1;
            UpdateINodeBitMap(iNodeBitMap, id);

            UpdateItem(dirBlockId, index, id);

            currentINode.length ++;
            time(&currentINode.lastChangeTime);
            UpdateINode(currentINode);

            AppendFCBLinkNode(currentFCBLink, *iNodePointer);
            delete iNodePointer;

            return 0;
        } else {
            printf("storage space is not enough, %d\n", blockId);
            return -1;
        }
    } else {
        printf("the directory can't append file item, %d\n", dirBlockId);
        return -1;
    }

}

void MyExt::UpdateResource() {

    rewind(myExtFilePointer);

    fwrite(&user, userSize, 1, myExtFilePointer);
    fwrite(&superBlock, superBlockSize, 1, myExtFilePointer);
    fwrite(blockBitMap, blockBitMapSize, 1, myExtFilePointer);
    fwrite(iNodeBitMap, iNodeBitMapSize, 1, myExtFilePointer);
    fclose(myExtFilePointer);

}

void MyExt::LogIn() {

    char userName[10];
    char passWord[33];

    while(1)
    {
        printf("username:");
        fgets(userName, sizeof(userName), stdin);
        if(userName[strlen(userName)-1] == '\n')
            userName[strlen(userName)-1] = '\0';

        system("stty -echo");
        printf("password:");
        fgets(passWord, sizeof(passWord), stdin);
        if(passWord[strlen(passWord)-1] == '\n')
            passWord[strlen(passWord)-1] = '\0';
        system("stty echo");

        strcat(passWord, salty);
        MD5 iMD5;
        iMD5.GenerateMD5((unsigned char *) passWord, strlen(passWord));
        memset(passWord, 0, strlen(passWord));
        strncpy(passWord, iMD5.ToString().c_str(), 32);
        passWord[32] = 0;
        printf("\n");
        if(strcmp(userName,user.userName)==0 && strcmp(passWord,user.passWord)==0)
            break;
        else
        {
            printf("username or password is not correct,please try again.\n");
        }
    }

}

void MyExt::LogOut() {

    UpdateResource();
    printf("%s has logout\n", user.userName);
    //重新打开系统并进行登录验证，由于命令循环未退出，故会继续接受命令
    OpenMyExt();
    LogIn();
}

void MyExt::ShowFileDigest(FCBLink pNode) {

    if(pNode == NULL)
        return;
    printf("%s",pNode->fcb.name);
    
    printf("\n");

}

void MyExt::ShowFileDetail(INodePointer iNodePointer) {

    if(iNodePointer == NULL) {
        return;
    }
    //format output
    if(iNodePointer->attr == 1) {
        printf("%c", 'd');
    } else {
        printf("%c", '-');
    }

    printf("%c", 'r');

    if(iNodePointer->type == 1) {
        printf("%c", 'w');
    } else {
        printf("%c", '-');
    }

    printf(" %10d", iNodePointer->length);
    printf(" %.12s", 4 + ctime(&(iNodePointer->createdTime)));
    printf(" %.12s", 4 + ctime(&(iNodePointer->lastChangeTime)));
    printf(" %s", iNodePointer->name);
    printf("\n");
}

void MyExt::StopHandle(int sig) {

    UpdateResource();
    exit(0);
}

void MyExt::Exit() {
    printf("Bye!\n");
    StopHandle(0);
}

int MyExt::Analyse(const char *str) {

    char cmd[5][20];
    for(int i = 0; i < 5; i++) {
        userCommand[i][0] = '\0';
        memset(cmd[i], 0, sizeof(cmd[i]));
    }

    sscanf(str, "%s %s %s %s %s",cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);

    for (int i = 0; i < 5; i++) {
        userCommand[i] = cmd[i];
    }

    for(int i = 1; i < LAST; i++)
    {
        if(userCommand[0] == systemCommand[i])
        {
            return i;
        }
    }
    return 0;
}

void MyExt::ShowPath() {

    printf("%s@localhost %s>",user.userName, currentPath.data());
}

int MyExt::Read(const char *fileName) {

    unsigned int id = FindChildINode(currentFCBLink, fileName);
    
    unsigned int itemCount = blockSize / itemSize;

    if (id) {
        INodePointer iNodePointer = new INode();

        GetINode(iNodePointer, id);
        if (iNodePointer->attr == iNodePointer->DIR) {

            printf("%s is a directory\n", fileName);
            return -1;
        }

        unsigned long len = iNodePointer->length;
        char * buff = new char[blockSize + 1];

        unsigned int blockId;

        for (int i = 0; i < DIRECT_INDEX; i++) {

            blockId = iNodePointer->addr[i];

            if (blockId) {
                
                memset(buff, 0, blockSize + 1);

                if (len > blockSize) {
                    len -= GetData(blockId, buff, (unsigned int) blockSize, 0);
                    printf("%s", buff);
                } else {
                    len -= GetData(blockId, buff, (unsigned int) len, 0);
                    printf("%s\n", buff);
                    return 0;
                }
            } else {

                printf("\n");
                delete[] buff;
                return 0;
            }

            if(len <= 0) {
                //read finish
                printf("\n");
                delete[] buff;
                return 0;
            }
        }
        
        unsigned int addrBlockId = iNodePointer->addr[SECOND_INDEX_POSITION];
        unsigned int * secondIndex = new unsigned int [itemCount + 1];
        
        if (addrBlockId) {
            
            GetSecondIndex(addrBlockId, secondIndex, blockSize);
            
            for (int i = 0; i < itemCount; i++) {
                
                blockId = secondIndex[i];
                
                if (blockId) {
                    
                    memset(buff, 0, blockSize + 1);
                    
                    if (len > blockSize) {
                        len -= GetData(blockId, buff, (unsigned int) blockSize, 0);
                        printf("%s", buff);
                    } else {
                        len -= GetData(blockId, buff, (unsigned int) len, 0);
                        printf("%s\n", buff);
                        delete[] buff;
                        delete[] secondIndex;
                        return 0;
                    }
                } else {
                    
                    printf("\n");
                    delete[] buff;
                    delete[] secondIndex;
                    return 0;
                }
                
                if(len <= 0) {
                    //read finish
                    printf("\n");
                    delete[] buff;
                    delete[] secondIndex;
                    return 0;
                }
            }
        }
        
        delete[] secondIndex;
    

    } else {
        printf("no such file or directory\n");
        return -1;
    }

    printf("Unexpected error!\n");
    return -1;
}

unsigned int MyExt::WaitForInput(char *buff, unsigned int limit) {

    unsigned long len = 0;

    char ch[3] = {0, 0, 0};

    while (len < limit) {

        ch[2] = (char) getchar();

        if(ch[0] == '<' && ch[1] == '/' && ch[2] == '>') {
            len -= 2;
            buff[len] = '\0';
            getchar();
            return len;
        } else {
            ch[0] = ch[1];
            ch[1] = ch[2];
            buff[len] = ch[2];
            len++;
        }
    }

    buff[len] = 0;
    return len;
}

void MyExt::OverWrite(INodePointer iNodePointer) {

    char * buff = new char[blockSize + 1];
    unsigned long num;
    unsigned int itemCount = blockSize / itemSize;
    unsigned long len = 0;

    unsigned int blockId;
    
    printf("write %s: use flag \"</>\" to end\n", iNodePointer->name);
    
    for (int i = 0; i < DIRECT_INDEX; i++) {
        blockId = iNodePointer->addr[i];

        if (blockId) {
            
            num = WaitForInput(buff, (unsigned int) blockSize);
            WriteData(blockId, buff, num, 0);
            len += num;
            
            iNodePointer->length = len;
            time(&(iNodePointer->lastChangeTime));
            UpdateINode(*iNodePointer);
            
            if (num < blockSize) {
                delete[] buff;
                return;
            }
            
        } else {
            
            blockId = GetAvailableBlockId();
            
            if (blockId) {
                
                superBlock.blockFree --;
                UpdateSuperBlock(superBlock);
                blockBitMap[blockId] = 1;
                UpdateBlockBitMap(blockBitMap, blockId);
                
                iNodePointer->addr[i] = blockId;
                time(&(iNodePointer->lastChangeTime));
                UpdateINode(*iNodePointer);
                
                num = WaitForInput(buff, (unsigned int) blockSize);
                WriteData(blockId, buff, num, 0);
                len += num;
                iNodePointer->length = len;
                UpdateINode(*iNodePointer);
            }
            
            if (num < blockSize) {
                delete[] buff;
                return;
            }
        }
    }
    
    unsigned int addrBlockId = iNodePointer->addr[SECOND_INDEX_POSITION];
    unsigned int * secondIndex = new unsigned int [itemCount + 1];
    
    if (addrBlockId) {
        
        GetSecondIndex(addrBlockId, secondIndex, blockSize);
        
        for (int i = 0; i < itemCount; i++) {
            
            blockId = secondIndex[i];
            
            if (blockId) {
                
                num = WaitForInput(buff, (unsigned int) blockSize);
                WriteData(blockId, buff, num, 0);
                len += num;
                
                iNodePointer->length = len;
                time(&(iNodePointer->lastChangeTime));
                UpdateINode(*iNodePointer);
                
                if (num < blockSize) {
                    delete[] buff;
                    delete[] secondIndex;
                    return;
                }
                
            } else {
                
                blockId = GetAvailableBlockId();
                
                if (blockId) {
                    
                    superBlock.blockFree --;
                    UpdateSuperBlock(superBlock);
                    blockBitMap[blockId] = 1;
                    UpdateBlockBitMap(blockBitMap, blockId);
                    
                    iNodePointer->addr[i] = blockId;
                    time(&(iNodePointer->lastChangeTime));
                    UpdateINode(*iNodePointer);
                    
                    num = WaitForInput(buff, (unsigned int) blockSize);
                    WriteData(blockId, buff, num, 0);
                    len += num;
                    iNodePointer->length = len;
                    UpdateINode(*iNodePointer);
                }
                
                if (num < blockSize) {
                    delete[] buff;
                    delete[] secondIndex;
                    return;
                }
            }
        }
    } else {
        
        addrBlockId = GetAvailableBlockId();
        
        if (addrBlockId) {
            
            superBlock.blockFree --;
            UpdateSuperBlock(superBlock);
            blockBitMap[addrBlockId] = 1;
            UpdateBlockBitMap(blockBitMap, addrBlockId);
            
            iNodePointer->addr[SECOND_INDEX_POSITION] = addrBlockId;
            time(&(iNodePointer->lastChangeTime));
            UpdateINode(*iNodePointer);
            
            GetSecondIndex(addrBlockId, secondIndex, blockSize);
            
            for (int i = 0; i < itemCount; i++) {
                
                blockId = secondIndex[i];
                
                if (blockId) {
                    
                    num = WaitForInput(buff, (unsigned int) blockSize);
                    WriteData(blockId, buff, num, 0);
                    len += num;
                    
                    iNodePointer->length = len;
                    time(&(iNodePointer->lastChangeTime));
                    UpdateINode(*iNodePointer);
                    
                    if (num < blockSize) {
                        delete[] buff;
                        delete[] secondIndex;
                        return;
                    }
                    
                } else {
                    
                    blockId = GetAvailableBlockId();
                    
                    if (blockId) {
                        
                        superBlock.blockFree --;
                        UpdateSuperBlock(superBlock);
                        blockBitMap[blockId] = 1;
                        UpdateBlockBitMap(blockBitMap, blockId);
                        
                        iNodePointer->addr[i] = blockId;
                        time(&(iNodePointer->lastChangeTime));
                        UpdateINode(*iNodePointer);
                        
                        num = WaitForInput(buff, (unsigned int) blockSize);
                        WriteData(blockId, buff, num, 0);
                        len += num;
                        iNodePointer->length = len;
                        UpdateINode(*iNodePointer);
                    }
                    
                    if (num < blockSize) {
                        delete[] buff;
                        delete[] secondIndex;
                        return;
                    }
                }
            }
        }
        
        if (num < blockSize) {
            delete[] buff;
            return;
        }
        
    }
    delete[] secondIndex;
    

}

void MyExt::AppendWrite(INodePointer iNodePointer) {
    
    char * buff = new char[blockSize + 1];
    unsigned long num;
    unsigned int itemCount = blockSize / itemSize;
    unsigned long len = iNodePointer->length;
    
    unsigned int blockId;
    
    Read(iNodePointer->name);
    printf("write %s: use flag \"</>\" to end\n", iNodePointer->name);
    
    for (int i = len / blockSize; i < DIRECT_INDEX; i++) {
        blockId = iNodePointer->addr[i];
        
        if (blockId) {
            
            unsigned int tempLen = 0;
            GetData(blockId, buff, (unsigned int) blockSize, 0);
            for (int i = 0; buff[i]; i++) {
                tempLen ++;
            }
            len -= tempLen;
            
            num = WaitForInput(buff + tempLen, (unsigned int) (blockSize - tempLen));
            
            num += tempLen;
            WriteData(blockId, buff, num, 0);
            
            len += num;
            
            iNodePointer->length = len;
            time(&(iNodePointer->lastChangeTime));
            UpdateINode(*iNodePointer);
            
            if (num < blockSize) {
                delete[] buff;
                return;
            }
        } else {
            
            blockId = GetAvailableBlockId();
            
            if (blockId) {
                
                superBlock.blockFree --;
                UpdateSuperBlock(superBlock);
                blockBitMap[blockId] = 1;
                UpdateBlockBitMap(blockBitMap, blockId);
                
                iNodePointer->addr[i] = blockId;
                time(&(iNodePointer->lastChangeTime));
                UpdateINode(*iNodePointer);
                
                num = WaitForInput(buff, (unsigned int) blockSize);
                WriteData(blockId, buff, num, 0);
                len += num;
                iNodePointer->length = len;
                UpdateINode(*iNodePointer);
            }
            
            if (num < blockSize) {
                delete[] buff;
                return;
            }
        }
    }
    
    unsigned int addrBlockId = iNodePointer->addr[SECOND_INDEX_POSITION];
    unsigned int * secondIndex = new unsigned int [itemCount + 1];
    
    if (addrBlockId) {
        
        GetSecondIndex(addrBlockId, secondIndex, blockSize);
        
        for (int i = (len / blockSize) - DIRECT_INDEX; i < itemCount; i++) {
            
            blockId = secondIndex[i];
            if (blockId) {
                
                unsigned int tempLen = 0;
                GetData(blockId, buff, (unsigned int) blockSize, 0);
                for (int i = 0; buff[i]; i++) {
                    tempLen ++;
                }
                len -= tempLen;
                
                num = WaitForInput(buff + tempLen, (unsigned int) (blockSize - tempLen));
                
                num += tempLen;
                WriteData(blockId, buff, num, 0);
                
                len += num;
                
                iNodePointer->length = len;
                time(&(iNodePointer->lastChangeTime));
                UpdateINode(*iNodePointer);
                
                if (num < blockSize) {
                    delete[] buff;
                    delete[] secondIndex;
                    return;
                }
            } else {
                
                blockId = GetAvailableBlockId();
                
                if (blockId) {
                    
                    superBlock.blockFree --;
                    UpdateSuperBlock(superBlock);
                    blockBitMap[blockId] = 1;
                    UpdateBlockBitMap(blockBitMap, blockId);
                    
                    iNodePointer->addr[i] = blockId;
                    time(&(iNodePointer->lastChangeTime));
                    UpdateINode(*iNodePointer);
                    
                    num = WaitForInput(buff, (unsigned int) blockSize);
                    WriteData(blockId, buff, num, 0);
                    len += num;
                    iNodePointer->length = len;
                    UpdateINode(*iNodePointer);
                }
                
                if (num < blockSize) {
                    delete[] buff;
                    delete[] secondIndex;
                    return;
                }
            }
        }
        
    } else {
        
        addrBlockId = GetAvailableBlockId();
        
        if (addrBlockId) {
            
            superBlock.blockFree --;
            UpdateSuperBlock(superBlock);
            blockBitMap[addrBlockId] = 1;
            UpdateBlockBitMap(blockBitMap, addrBlockId);
            
            iNodePointer->addr[SECOND_INDEX_POSITION] = blockId;
            time(&(iNodePointer->lastChangeTime));
            UpdateINode(*iNodePointer);
            
            GetSecondIndex(addrBlockId, secondIndex, blockSize);
            
            for (int i = (len / blockSize) - DIRECT_INDEX; i < itemCount; i++) {
                
                blockId = secondIndex[i];
                if (blockId) {
                    
                    unsigned int tempLen = 0;
                    GetData(blockId, buff, (unsigned int) blockSize, 0);
                    for (int i = 0; buff[i]; i++) {
                        tempLen ++;
                    }
                    len -= tempLen;
                    
                    num = WaitForInput(buff + tempLen, (unsigned int) (blockSize - tempLen));
                    
                    num += tempLen;
                    WriteData(blockId, buff, num, 0);
                    
                    len += num;
                    
                    iNodePointer->length = len;
                    time(&(iNodePointer->lastChangeTime));
                    UpdateINode(*iNodePointer);
                    
                    if (num < blockSize) {
                        delete[] buff;
                        delete[] secondIndex;
                        return;
                    }
                } else {
                    
                    blockId = GetAvailableBlockId();
                    
                    if (blockId) {
                        
                        superBlock.blockFree --;
                        UpdateSuperBlock(superBlock);
                        blockBitMap[blockId] = 1;
                        UpdateBlockBitMap(blockBitMap, blockId);
                        
                        iNodePointer->addr[i] = blockId;
                        time(&(iNodePointer->lastChangeTime));
                        UpdateINode(*iNodePointer);
                        
                        num = WaitForInput(buff, (unsigned int) blockSize);
                        WriteData(blockId, buff, num, 0);
                        len += num;
                        iNodePointer->length = len;
                        UpdateINode(*iNodePointer);
                    }
                    
                    if (num < blockSize) {
                        delete[] buff;
                        delete[] secondIndex;
                        return;
                    }
                }
            }
        }
        
        if (num < blockSize) {
            delete[] buff;
            return;
        }
    }
    delete[] secondIndex;
}

int MyExt::Write(const char *fileName, unsigned int method) {

    unsigned int id = FindChildINode(currentFCBLink, fileName);

    if (id) {

        INodePointer iNodePointer = new INode();
        GetINode(iNodePointer, id);

        if (iNodePointer->type == iNodePointer->READ_ONLY) {
            printf("file %s is Read-Only file\n", fileName);
            return -1;
        }

        if (method == OVER_WRITE) {
            OverWrite(iNodePointer);
            return 0;
        } else if (method == APPEND) {
            AppendWrite(iNodePointer);
            return 0;
        } else {
            printf("Wrong write method, please use -o -a\n");
            return -1;
        }
    } else {
        printf("no such file or directory\n");
        return -1;
    }

    printf("Unexpected error!\n");
    return -1;
}

void MyExt::Del(const char * fileName) {
    
    unsigned int id = FindChildINode(currentFCBLink, fileName);
    
    if (id) {
        
        INodePointer iNodePointer = new INode();
        
        GetINode(iNodePointer, id);
        if (iNodePointer->attr == iNodePointer->FILE || iNodePointer->length <= 0) {
            
            unsigned int blockId;
            
            for (int i = 0; i < DIRECT_INDEX; i++) {
                blockId = iNodePointer->addr[i];
                
                if (blockId) {
                    superBlock.blockFree ++;
                    UpdateSuperBlock(superBlock);
                    
                    ReleaseBlock(blockId);
                    
                    blockBitMap[blockId] = 0;
                    UpdateBlockBitMap(blockBitMap, blockId);
                }
            }
            
            ReleaseINode(iNodePointer->id);
            currentINode.length --;
            UpdateINode(currentINode);
            
            superBlock.iNodeNum --;
            UpdateSuperBlock(superBlock);
            
            ReleaseBlock(iNodePointer->blockId);
            blockBitMap[iNodePointer->blockId] = 0;
            UpdateINodeBitMap(blockBitMap, iNodePointer->blockId);
            
            iNodeBitMap[iNodePointer->id] = 0;
            UpdateINodeBitMap(iNodeBitMap, iNodePointer->id);
            
            ReleaseItem(iNodePointer->parent, iNodePointer->id);
            RemoveFCBLinkNode(currentFCBLink, fileName);
        } else {
            Cd(fileName);
            
            FCBLink link = currentFCBLink->next;
            
            while (link) {
                Del(link->fcb.name);
                link = link->next;
            }
            Cd("..");
            Del(fileName);
        }
        
        delete iNodePointer;
    }
    
}

void MyExt::SystemInfo() {
    printf("Sum of block number:%d\n",superBlock.blockNum);
    printf("Each block size:%d\n",superBlock.blockSize);
    printf("Free of block number:%d\n",superBlock.blockFree);
    printf("Sum of inode number:%d\n\n",superBlock.iNodeNum);
}

void MyExt::Clr() {
    
    system("clear");
}
