#ifndef  GETFILELIST_H
#define  GETFILELIST_H


#include <stdint.h>

#define MAX_LENGTH 512
#define MAX_FILENAME 64
#define onesector 512 //扇区512 

// 数字合并函数
uint16_t uint8to16(uint8_t twouint8[2]) {
	return *(uint16_t*)twouint8;
}

uint32_t uint8to32(uint8_t fouruint8[4]) {
	return *(uint32_t*)fouruint8;
}

uint64_t uint8to64(uint8_t eightuint8[8]) {
	return *(uint64_t*)eightuint8;
}
// 结构体声明
//DBR结构，EBR应该也是类似管理 
struct DBR {
	uint8_t jumpcode[3];//EB 58 90
	uint8_t OEM[8];//OEM代号
	uint8_t bytes_per_sector[2];//扇区字节数
	uint8_t secotrs_per_cluster;//每簇扇区数
	uint8_t reserve_sectors[2];//包括DBR自己在内的FAT之前的扇区个数
	uint8_t FATnum;//FAT个数，一般为2 
	uint8_t unimportant1[11];
	uint8_t DBR_LBA[4];//该分区的DBR所在的相对扇区号，如果是扩展分区，是相对于扩展分区首的
	uint8_t totalsectors[4];//本分区的总扇区数
	uint8_t sectors_per_FAT[4];//每个FAT的扇区数
	uint8_t unimportant2[4];
	uint8_t root_cluster_number[4];//根目录簇号
	uint8_t file_info[2];
	uint8_t backup_DBR[2];//备份引导扇区的相对于DBR的扇区号，一般为6，内容和DBR一模一样
	uint8_t zero1[12];
	uint8_t extBPB[26];//扩展BPB
	uint8_t osboot[422];//引导代码和55AA 
};

// typedef struct fileClusterChain //文件簇链
// {
//     unsigned int clusterNum;
//     struct fileClusterChain *next;
// }fileClusterChain;

// typedef struct fileNameList //文件名链表
// {
//     char fileName[MAX_LENGTH]; //文件名
//     fileClusterChain *clusterChain; //对应簇链
//     struct fileNameList *next;  //下一个文件
// }fileNameList;
typedef struct spiltedPath //分割后的路径
{
    char *path;
	// int clusterNum;
    struct spiltedPath *next;
}spiltedPath;
typedef struct clusterNumList{	// 簇号列表
	int clusterNum;
	struct clusterNumList *next;
}clusterNumList;
typedef struct cluster	//簇内所有信息
{
	uint8_t *data;
	struct clusterNumList *clusterNumList;
}cluster;
typedef struct fileNameList //文件名链表
{
    char fileName[MAX_LENGTH]; //文件名
    // fileClusterChain *clusterChain; //对应簇链
    struct fileNameList *next;  //下一个文件
}fileNameList;
typedef struct fileData{	// 文件数据,字节流
	char *data;
	int startClusterNum;
	struct fileData *next;
}fileData;
typedef struct ShortDirItem{
    char filename[8];//第一部分文件名
    char extname[3];//文件扩展名
    uint8_t attr;//属性 0F则说明是长文件需要索引到非0F，然后倒着读回来
    uint8_t reserve;
    uint8_t time1;
    uint8_t creattime[2];
    uint8_t createdate[2];
    uint8_t visittime[2];
    uint16_t high_cluster;//文件起始簇号高16位
    uint8_t changetim2[2];
    uint8_t changedate[2];
    uint16_t low_cluster;//文件起始簇号低16位
    uint8_t filelen[4];//文件长度
}ShortDirItem;
typedef struct LongDirItem {
    uint8_t order;
    uint16_t name1[5];
    uint8_t attribute;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t firstCluster;
    uint16_t name3[2];
} LongDirItem;
typedef struct fileNameItem {
    int isLong;
    ShortDirItem shortItem;
    LongDirItem longItem;
}fileNameItem;
typedef struct fileNameItemChain {
    fileNameItem dirItem;
    struct fileNameItemChain *next;
}fileNameItemChain;
typedef struct fullNameList{
    char name[64];
    int firstCluster;
    struct fullNameList *next;
}fullNameList;

// 函数声明
boolean findfile(fileNameList* name);
spiltedPath *spiltPath(char *path); //分割路径
fullNameList *getFullNameList(fileNameItemChain *longFileName);
char *uint8_to_char(uint8_t *uint8, int len); //uint8转char
void processStr(char *str); //删除字符间空格 
uint32_t uint16_to32(uint16_t high, uint16_t low); //uint16转uint32
uint8_t uint16to8_(uint16_t num);
void initlpBuffer();// 初始化lpBuffer,全为0
void show_bytes(uint8_t* start, int len); //显示字节,十六进制，调试用
void strReverse(char *str, int start, int end); //翻转字符串
fileNameItemChain *getDirItemChain(uint8_t *cluster, int clusterLen); //依据目录簇内的数据，获取目录项链表
// 输入：DirItemChain，输出：长目录项DirItemChain
fileNameItemChain *getLongDirItemChain(fileNameItemChain *dirItemChain); //获取长目录项链表,未使用
// TODO: 读取文件簇链
uint32_t getFirstCluser(uint32_t currentCluster, char* name, int len, HANDLE hDevice, uint32_t start, uint32_t dataStart);  //获取文件的第一个簇号
    // 根据当前第一个簇号，读取所有簇，并在簇中找出name对应的第一个簇号,用于查找文件
fullNameList *getFullNameList(fileNameItemChain *longFileName); //把输入处理成名字加起始簇号的链表，其中长目录项会被拼接
int getClusterChain(char* fileName); //获取文件簇链
spiltedPath *spiltPath(char *path); //分割路径
uint8_t uint16to8_(uint16_t num); //uint16转uint8,取低位

#endif
