#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wtypes.h>
#include "FAT32-reader.h"

#define DEBUG
#define size_cluster 4096
#define size_sector 512
// typedef char uint8_t;
uint8_t lpBuffer[onesector] = { 0 };//用于读size_cluster字节 

void processStr(char *str){ //删除字符间空格 
    int i = 0;
    int j = 0;
    int len = strlen(str);
    for (i = 0; i < len; i++){
        if (str[i] != ' '){
            str[j] = str[i];
            j++;
        }
        if(str[i]=='\377'){
            str[j] = '\0';
            break;
        }
    }

    if(str[j - 1]=='.' || str[j - 1]==-1){
        str[j - 1] = '\0';
    }
    str[j] = '\0';
}
uint32_t uint16_to32(uint16_t high, uint16_t low) {
    uint32_t result = 0;
    result = high;
    result = result << 16;
    result = result | low;
    return result;
}
uint8_t uint16to8_(uint16_t num);
void initlpBuffer() {
    memset(lpBuffer, 0, onesector);
}
void show_bytes(uint8_t* start, int len) {
     int i;
     len /= 10;
     for (i = 0; i < len; i++)
        printf(" %02x", start[i]);    //line:data:show_bytes_printf
     printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s [path]", argv[0]);
        return 1;
    }
    char path[MAX_LENGTH] = { 0 };
    strcpy(path, argv[1]);

    // char path[MAX_LENGTH] = "H:\\happy\\game\\twdassadbasdbaasdjasda\\nothinsdfsdfg.txt";
    // char path[MAX_LENGTH] = "H:\\happy\\game\\test.tt";

    getClusterChain(path);
    return 0;
}

void strReverse(char *str, int start, int end) //翻转字符串
{
    char temp;
    while (start < end)
    {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}
fileNameItemChain *getDirItemChain(uint8_t *cluster, int clusterLen){
    fileNameItem *p1 = (fileNameItem *)malloc(sizeof(fileNameItem));
    fileNameItemChain *dirItems = (fileNameItemChain *)malloc(sizeof(fileNameItemChain));
    // dirItems = (DirItemChain *)malloc(sizeof(DirItemChain));
    fileNameItemChain *p = dirItems;
#ifdef DEBUG
    printf("fileNameItemChain clusterLen: %d\n", clusterLen);
    show_bytes(cluster, clusterLen);
#endif
    int i = 0;
    while (i < clusterLen){
        if (cluster[i] == 0x00) break;
        if (cluster[i] == 0xE5) {
            i += 32;
            continue;
        }
        if (cluster[i + 11] == 0x0F) {
            p->dirItem.isLong = 1;
            // memcpy(&p->dirItem.longItem, cluster + i, 32);
            p->dirItem.longItem.order = cluster[i];
            uint8_t temp[2] ={cluster[i+1], cluster[i+2]};
            p->dirItem.longItem.name1[0] = uint8to16(temp);
            temp[0] = cluster[i+3];temp[1]=cluster[i+4];
            p->dirItem.longItem.name1[1] = uint8to16(temp);
            temp[0] = cluster[i+5];temp[1]=cluster[i+6];
            p->dirItem.longItem.name1[2] = uint8to16(temp);
            temp[0] = cluster[i+7];temp[1]=cluster[i+8];
            p->dirItem.longItem.name1[3] = uint8to16(temp);
            temp[0] = cluster[i+9];temp[1]=cluster[i+10];
            p->dirItem.longItem.name1[4] = uint8to16(temp);
            p->dirItem.longItem.attribute= cluster[i+11];
            p->dirItem.longItem.type = cluster[i+12];
            p->dirItem.longItem.checksum = cluster[i+13];
            temp[0] = cluster[i+14];temp[1]=cluster[i+15];
            p->dirItem.longItem.name2[0] = uint8to16(temp);
            temp[0] = cluster[i+16];temp[1]=cluster[i+17];
            p->dirItem.longItem.name2[1] = uint8to16(temp);
            temp[0] = cluster[i+18];temp[1]=cluster[i+19];
            p->dirItem.longItem.name2[2] = uint8to16(temp);
            temp[0] = cluster[i+20];temp[1]=cluster[i+21];
            p->dirItem.longItem.name2[3] = uint8to16(temp);
            temp[0] = cluster[i+22];temp[1]=cluster[i+23];
            p->dirItem.longItem.name2[4] = uint8to16(temp);
            temp[0] = cluster[i+24];temp[1]=cluster[i+25];
            p->dirItem.longItem.name2[5] = uint8to16(temp);
            temp[0] = cluster[i+26];temp[1]=cluster[i+27];
            p->dirItem.longItem.firstCluster = uint8to16(temp);
            temp[0] = cluster[i+28];temp[1]=cluster[i+29];
            p->dirItem.longItem.name3[0] = uint8to16(temp);
            temp[0] = cluster[i+30];temp[1]=cluster[i+31];
            p->dirItem.longItem.name3[1] = uint8to16(temp);

            
            i+=32;
            p->next = (fileNameItemChain *)malloc(sizeof(fileNameItemChain));
            p = p->next;
            continue;
        }
        p->dirItem.isLong = 0;
        memcpy(&p->dirItem.shortItem, cluster + i, 32);
#ifdef DEBUG
        printf("fileNameItemChain shortItem: %s\n", p->dirItem.shortItem.filename);
#endif
        i += 32;
        p->next = (fileNameItemChain *)malloc(sizeof(fileNameItemChain));
        p = p->next;
    }
    p->next = NULL;
    return dirItems;
}
uint32_t getFirstCluser(uint32_t currentCluster, char* name, int len, HANDLE hDevice, uint32_t start, uint32_t dataStart){  //获取文件的第一个簇号
    // 根据当前第一个簇号，读取所有簇，并在簇中找出name对应的第一个簇号
    clusterNumList cnl;
    clusterNumList *cnlp = &cnl;
    cnlp->clusterNum = currentCluster;
    cnlp->next = (clusterNumList *)malloc(sizeof(clusterNumList));
    fileNameItem *p1 = (fileNameItem *)malloc(sizeof(fileNameItem));
    cnlp = cnlp->next;
    cnlp->next = NULL;
    int clusterNum = 0xfffffff;
    int i = 0;
    DWORD dwCB;
    do  //读取簇链
    {
        SetFilePointer(hDevice, start , NULL, FILE_BEGIN);// 读取FAT表项
        initlpBuffer();
	    ReadFile(hDevice, lpBuffer, size_cluster, &dwCB, NULL); // ipBuffer是一个缓冲区，用来存放读取的数据
        uint8_t temp[4]={lpBuffer[0+currentCluster*4],lpBuffer[1+currentCluster*4],lpBuffer[2+currentCluster*4],lpBuffer[3+currentCluster*4]};
        currentCluster = uint8to64(temp);
        if (currentCluster == 0x0fffffff){
            break;
        }
#ifdef DEBUG
        printf("currentCluster: %x\n", currentCluster);
#endif
        cnlp->clusterNum = currentCluster;
        cnlp->next = (clusterNumList *)malloc(sizeof(clusterNumList));
        cnlp = cnlp->next;
        cnlp->next = NULL;
        i++;
    } while (clusterNum != 0xfffffff);
    // malloc(128);
    uint8_t *fileData = (uint8_t *)malloc((i+1) * size_cluster);
    // malloc(128);
    uint8_t *fileDataP = fileData;
    cnlp = &cnl;
    while(cnlp->next != NULL && cnlp->clusterNum != 0xfffffff){  //读取该文件的所有簇
        SetFilePointer(hDevice, dataStart + cnlp->clusterNum * size_cluster, NULL, FILE_BEGIN);// 读取FAT表项
        initlpBuffer();
        ReadFile(hDevice, lpBuffer, size_cluster, &dwCB, NULL); // ipBuffer是一个缓冲区，用来存放读取的数据
#ifdef DEBUG
        printf("%d\n", dataStart + cnlp->clusterNum * size_cluster);
        show_bytes(lpBuffer, size_cluster);
#endif
        memcpy(fileDataP, lpBuffer, size_cluster);
        fileDataP += size_cluster;
        malloc(128);
        cnlp = cnlp->next;
    }
    // fileNameItem *p1 = (fileNameItem *)malloc(sizeof(fileNameItem));
    // malloc(128);
    fileNameItemChain *dic = getDirItemChain(fileData, (i+1) * size_cluster);
    fullNameList *fnl = getFullNameList(dic);//FIXME
    uint32_t firstCluster = 0;
    while(fnl != NULL){
#ifdef DEBUG
        printf("fnl->fullName: %s\n", fnl->name);
        printf("name         : %s\n", name);
#endif
        if (strcasecmp(fnl->name, name) == 0){
            firstCluster = fnl->firstCluster;
            break;
        }
        fnl = fnl->next;
    }
    if (firstCluster == 0){
        printf("文件不存在");
        return 0;
    }
    return firstCluster;
}
fullNameList *getFullNameList(fileNameItemChain *longFileName){
    fullNameList *fNL = (struct fullNameList *)malloc(sizeof(fullNameList));
    #ifdef DEBUG
    printf("sizeof fullnameList %d\n", sizeof(fNL));
    #endif
    memset(fNL, 0, sizeof(fNL));
    struct fullNameList *p = fNL;
    fileNameItemChain *q = longFileName;
    while (q ->next!= NULL){
        if(q->dirItem.isLong){ //倒着写到最近的一个短目录项
            int i=0;
            while(q != NULL && q->dirItem.isLong){
                p->name[13*i+0] = uint16to8_(q->dirItem.longItem.name1[0]);
                p->name[13*i+1] = uint16to8_(q->dirItem.longItem.name1[1]);
                p->name[13*i+2] = uint16to8_(q->dirItem.longItem.name1[2]);
                p->name[13*i+3] = uint16to8_(q->dirItem.longItem.name1[3]);
                p->name[13*i+4] = uint16to8_(q->dirItem.longItem.name1[4]);
                p->name[13*i+5] = uint16to8_(q->dirItem.longItem.name2[0]);
                p->name[13*i+6] = uint16to8_(q->dirItem.longItem.name2[1]);
                p->name[13*i+7] = uint16to8_(q->dirItem.longItem.name2[2]);
                p->name[13*i+8] = uint16to8_(q->dirItem.longItem.name2[3]);
                p->name[13*i+9] = uint16to8_(q->dirItem.longItem.name2[4]);
                p->name[13*i+10] = uint16to8_(q->dirItem.longItem.name2[5]);
                p->name[13*i+11] = uint16to8_(q->dirItem.longItem.name3[0]);
                p->name[13*i+12] = uint16to8_(q->dirItem.longItem.name3[1]);

                strReverse(p->name, 13*i, 13*i+12);
                // processStr(p->name);
                i++;
                // p->firstCluster = q->dirItem.longItem.firstCluster;
                // p->next = (fullNameList *)malloc(sizeof(fullNameList));
                // p = p->next;
                q = q->next;
            }
            strReverse(p->name, 0, 13*i - 1);
            p->name[13*i] = '\0';
            processStr(p->name);
            #ifdef DEBUG
            printf("long name: %s\n", p->name);
            #endif
            // uint16_t tt ={q->dirItem.shortItem->firstClusterHigh, q->dirItem.shortItem->firstClusterLow};
            uint32_t cl= uint16_to32(q->dirItem.shortItem.high_cluster, q->dirItem.shortItem.low_cluster);
            p->firstCluster = cl; // FIXME
            p->next = (struct fullNameList *)malloc(sizeof(fullNameList));
            p = p->next;
        }
        else{   //短目录项
            // p->name = (char *)malloc(12);
            memcpy(p->name, q->dirItem.shortItem.filename, 8);
            p->name[8] = '.';
            memcpy(p->name + 9, q->dirItem.shortItem.extname, 3);
            p->name[12] = '\0';
            // uint16_t cl= uint8to16(q->dirItem.shortItem.low_cluster);
            uint32_t cl= uint16_to32(q->dirItem.shortItem.high_cluster, q->dirItem.shortItem.low_cluster);
            p->firstCluster = cl; // FIXME
            processStr(p->name);
#ifdef DEBUG
            printf("name = %s\n", p->name);
            printf("firstCluster = %x\n", cl);
#endif
            p->next = (struct fullNameList *)malloc(sizeof(fullNameList));
            p = p->next;
            q = q->next;
        }
    }
    return fNL;
}
int getClusterChain(char* fileName) //获取文件簇链
{
    FILE *fp = fopen(fileName, "rb");
    if (fp == NULL)
    {
        printf("No such a file %s failed!", fileName);
        return 1;
    }
    else
    {   
        DISK_GEOMETRY pdg;            // 保存磁盘参数的结构体
        BOOL bResult;                 // generic results flag
        ULONGLONG DiskSize;           // size of the drive, in bytes
        HANDLE hDevice;               // 设备句柄
        DWORD junk;                   // discard resultscc

        // fileClusterChain *clusterChain = (fileClusterChain *)malloc(sizeof(fileClusterChain)); //分配内存
        // fileClusterChain *p = clusterChain;
        spiltedPath *fullPath = spiltPath(fileName);    //把路径分割,同时获取分区信息
        printf("fullPath->partition = %s\n", fullPath->path);
        // hDevice = CreateFile(fullPath->path, // 设备名称,这里指硬盘甚至可以是分区/扩展分区名，不区分大小写 
		// GENERIC_READ,                // 读
		// FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
		// NULL,             // default security attributes
		// OPEN_EXISTING,    // disposition
		// 0,                // file attributes
		// NULL);            // do not copy file attributes
        hDevice = CreateFile(TEXT("\\\\.\\H:"), // 设备名称,这里指硬盘甚至可以是分区/扩展分区名，不区分大小写
        GENERIC_READ,                // 读
        FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
        NULL,             // default security attributes
        OPEN_EXISTING,    // disposition
        0,                // file attributes
        NULL);            // do not copy file attributes

        if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
        {
            printf("May be no permission!Or no such partition!\n");
            return 1;
        }
	    //通过DeviceIoControl函数与设备进行IO，为后面做准备 
	    bResult = DeviceIoControl(hDevice, // 设备的句柄
		IOCTL_DISK_GET_DRIVE_GEOMETRY, // 控制码，指明设备的类型
		NULL, 0, // no input buffer
		&pdg,
		sizeof(pdg),     // output buffer 输出，保存磁盘参数信息
		&junk,                 // # bytes returned
		(LPOVERLAPPED)NULL); // synchronous I/O

        LARGE_INTEGER offset;//读取位置 
        offset.QuadPart = (ULONGLONG)0;//0
        SetFilePointer(hDevice, 0, NULL, FILE_BEGIN);//从这个位置开始读，DBR是FILE_BEGIN，相对位移！！！ 

        DWORD dwCB; //读取的字节数
        SetFilePointer(hDevice, 0, NULL, FILE_BEGIN);
        // memset(lpBuffer, 0, sizeof(lpBuffer));
        initlpBuffer();
        BOOL bRet = ReadFile(hDevice, lpBuffer, size_cluster, &dwCB, NULL);
#ifdef DEBUG
        show_bytes(lpBuffer, size_cluster);
#endif
        uint8_t len[2] = { lpBuffer[14], lpBuffer[15] };    
		uint16_t lenOfMBR = uint8to16(len);
        uint8_t len2[4] = {lpBuffer[36], lpBuffer[37], lpBuffer[38], lpBuffer[39]};
        uint32_t lenOfFAT = uint8to32(len2);
		int offsetOfFAT = lenOfMBR * size_sector;//FAT1的偏移量
        uint32_t offsetOfData = ((uint32_t)lenOfMBR + lenOfFAT * 2) * size_sector - size_cluster * 2;//数据区的偏移量

#ifdef DEBUG
        printf("lenOfMBR:%d\n", lenOfMBR);
        printf("lenOfFAT:%d\n", lenOfFAT);
        printf("offsetOfFAT:%x\n", offsetOfFAT);
        printf("offsetOfData:%x\n", offsetOfData);
#endif

        SetFilePointer(hDevice, offsetOfFAT, NULL, FILE_BEGIN);// 读取FAT表项
        initlpBuffer();
		bRet = ReadFile(hDevice, lpBuffer, size_cluster, &dwCB, NULL); // ipBuffer是一个缓冲区，用来存放读取的数据
#ifdef DEBUG
        printf("FAT1:\n");
        show_bytes(lpBuffer, size_cluster);
#endif
       
        spiltedPath *p_path = fullPath;
        fileNameItem *p1 = (fileNameItem *)malloc(sizeof(fileNameItem));
        p_path = p_path->next;
        malloc(128);
        uint32_t firstCluster = getFirstCluser(2,p_path->path,strlen(p_path->path),  hDevice,offsetOfFAT, offsetOfData);
        while(p_path->next!=NULL) // 一层一层目录去找
        {
            p_path=p_path->next;
            if(p_path->next==NULL)
            {
                break;
            }
            firstCluster=getFirstCluser(firstCluster,p_path->path,strlen(p_path->path),hDevice,offsetOfFAT, offsetOfData); 
            // p_path=p_path->next;
        }
        // 找到了文件的第一个簇号
        // 从这个簇号开始读，读到文件尾
       
        int clusterNum = 0xfffffff;
        int i = 0;
        uint32_t currentCluster = firstCluster;
        // DWORD dwCB;
        do  //读取簇链
        {
            printf("currentCluster: %d\n", currentCluster);
            SetFilePointer(hDevice, offsetOfFAT , NULL, FILE_BEGIN);// 读取FAT表项
            initlpBuffer();
            ReadFile(hDevice, lpBuffer, size_cluster, &dwCB, NULL); // ipBuffer是一个缓冲区，用来存放读取的数据
            uint8_t temp[4]={lpBuffer[0+currentCluster*4],lpBuffer[1+currentCluster*4],lpBuffer[2+currentCluster*4],lpBuffer[3+currentCluster*4]};
            currentCluster = uint8to64(temp);
            if (currentCluster == 0x0fffffff){
                break;
            }
    #ifdef DEBUG
            // printf("currentCluster: %x\n", currentCluster);
    #endif
        } while (currentCluster != 0xfffffff);
        // 读取完毕，开始写入文件
        return 1;
    }
}
spiltedPath *spiltPath(char *path) //分割路径
{
    spiltedPath *sp = (spiltedPath *)malloc(sizeof(spiltedPath));
    char *p = path;
    // p+=2;
    spiltedPath *q = sp;
    q->path = path;
    q->next = (spiltedPath *)malloc(sizeof(spiltedPath));
    q = q->next;
    while (*p != '\0'){
        if(*p == ':'){
            *p = '\0';
            q->path = p+2;
            q->next = (spiltedPath *)malloc(sizeof(spiltedPath));
            q = q->next;
            p++;
        }
        else if (*p == '\\' || *p == '/'){
            *p = '\0';
            q->path = p + 1;
            q->next = (spiltedPath *)malloc(sizeof(spiltedPath));
            q = q->next;    // 获取一段路径
        }
        p++;
    }
    q->next = NULL;
    char partition[10] = "\\\\.\\C:"; // 处理盘符
    partition[4] = sp->path[0];
    sp->path = partition;
#ifdef DEBUG
    printf("%s\n", sp->path);
#endif
    return sp;
}
uint8_t uint16to8_(uint16_t num)
{
    uint8_t temp[2] = { 0 };
    temp[0] = num & 0xff;
    temp[1] = (num >> 8) & 0xff;
    if(temp[0] == -1)
        return ' ';
    if(temp[0] == 0 )
        return ' ';
    return temp[0];
}