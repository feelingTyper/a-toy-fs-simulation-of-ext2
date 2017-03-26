#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include "windows.h"
#define TEST 100
#define BlkSize 1024
#define InodeNum 512
#define BlkNum 1024
#define MaxBlkPerFile 16
#define MaxFileSize (MaxBlkPerFile*BlkSize)
#define BuffSize MaxFileSize
#define DiskSize (BlkSize*BlkNum + InodeNum*sizeof(Inode) + sizeof(SuperBlk))
#define RootUser "lshaluminum"
#define PassWord "lshaluminum"
#define CommandNum (sizeof(command)/sizeof(char*))
#define DISK "Disk"
#define SuperBlkBase 0
#define InodeBlkBase (sizeof(SuperBlk))
#define DataBlkBase (InodeBlkBase + (sizeof(Inode)*InodeNum))
#define MaxItemPerDir (BlkSize/sizeof(FCB))
#define MaxDirItemPerBlk (BlkSize/sizeof(FCB))
#define Dictionary 0
#define File 1
#define Link 2
#define Pipe 3
typedef struct
{
    int inode_map[InodeNum];//inode使用位图
    int blk_map[BlkNum];//block使用情况位图
    int inode_used;//inode使用统计
    int blk_used;//block使用统计
} SuperBlk;

typedef struct
{
    int type;//文件类型
    int blk_num;//文件占用数据块数量
    int file_size;//文件大小
    int file_blk_rank[MaxBlkPerFile];//文件数据块分布图
    char mode[4];//文件权限
    char owner[32];//文件创建者
    char create_time[64];//文件创建时间
    char modified_time[64];//文件修改时间
} Inode;

typedef struct
{
    char filename[32];//文件名
    int inode_rank;//文件对应的inode号
} FCB; //目录文件block中存放的内容

struct CurrentDir//当前工作的目录
{
    int dir_num;//当前目录文件的目录项数目
    int inode_rank;//当前目录文件的inode号
    Inode curr_inode;//当前目录文件的inode的内容
    char curr_path[128];//当前的路径
    FCB dir_item[MaxItemPerDir];//当前目录文件的内容
} CurentDir;

struct OpFile //当前打开的普通文件
{
    int inode_rank;//当前打开文件的inode节点号
    Inode inode;//当前打开文件的详细信息，即其对应的inode的内容
    char* buff;//缓冲区
} OpFile = {0};//初始化为0，防止出现野指针
SuperBlk super_blk;
FILE* Disk;

//文件系统使用函数列表
int init_fs();//初始化文件系统，主要加载根文件
int fmt_fs();//格式化文件系统
int exit_fs();//退出文件系统
int list_file();//查看文件目录项
int move_file();//移动/重命名文件 未实现
int copy_file();//拷贝文件 未实现
int close_file();//关闭打开的文件
int show_disk();//查看虚拟硬盘的使用情况
int logout_fs();//登出文件系统
int apply_blk();//申请数据块
int open_dir(int);//打开目录
int change_dictionary(char* );//切换目录
int remove_file(char* , int );//删除文件
int create_file(char* , int );//创建文件
int open_file(char*);//打开文件
int write_file(char* );//写文件
int type_check(char*);//文件类型检查
int apply_inode();//申请inode节点
int check_rename(char*);//检查重命名
void set_time(char*);//设置时间
void init_inode(int, int);//对申请的inode进行初始化
void rewrite_dir();//将当前目录内容重新写进硬盘
int get_inode_rank(char*);//获得inode编号
void rewrite_superblk();//将superblock重新写进硬盘
void free_file_blk(int);//释放数据块
void free_file_inode(int);//释放inode节点
void adjust_dir(int);//调整当前的目录内容
int is_empty_dictionary(int);//判断是否为空文件夹
int read_file(char* );//读文件
int write_file_buff(char* );//将buff中的内容写进文件中
void show_map(int, int*, int);//查看inode节点/数据块使用分布图
void c_printf(const char* );//颜色打印
void c_printf(char* );//颜色打印
void show_command();
void adduser(char* username);
void chmod(char* filename, int mode);
void chown(char* username);
void newpassword(char* password);
void ln(char* lname, char* filename);
void userdel(char* username);
void set_console_color(unsigned short color_index);

//支持的操作，目前还有一些没有实现
const char* command[] = {"fmt","exit","mkdir","ls","cd","rm","touch","mv","help",
"read","blkmap","inodemap","write","logout","fdisk","adduser","chmod","chown","password","ln","userdel"};
const char* command_help[] =
{
    "help",
    "fmt",
    "ln [-s] [lnfilename] [filename]",
    "chmod [-R] [wrx] [filename]",
    "exit",
    "adduser [username]",
    "userdel [username]",
    "password [password]",
    "chown [username]",
    "mkdir [filename]",
    "ls [-l] [filename]",
    "cd [dictionary name]",
    "rm [filename]",
    "touch [filename]",
    "mv [filename] [newfilename]",
    "read [filename]",
    "blkmap",
    "inodemap",
    "write [filename]",
    "logout",
    "fdisk"
};
char username[32] = "\0";
char password[64] = "\0";
int isLogin = 0;
int main(void)
{
    set_console_color(11);
    int i = 0, quit = 0;
    int select = -1;
    char c, date[64];
    char cmd[128] = "\0";
    char name[32];
    memset(cmd,'\0',sizeof(cmd));
    printf("------------------------------------------------------------\n");
    printf("\tFilsystem Version 1.0\n");
    printf("------------------------------------------------------------\n");
    Disk = fopen(DISK,"rb+");
    if (!Disk) {
        Disk = fopen(DISK,"wb+");
        fmt_fs();
    }
    init_fs();
login:
#ifndef TEST
    while (1) {
        printf("Username: ");
        while (((c = getchar()) != '\n') && (i < 32))
                username[i++] = c;
        printf("Password: ");
//        system("stty -echo");
        i = 0;
        while (((c = getchar()) != '\n') && (i < 64))
                password[i++] = c;
//        system("stty echo");
        if (!strcmp(username, RootUser) && !strcmp(password, PassWord)) {
            isLogin = 1;
            set_time(date);
            printf("\033[1;32m\nWelcome %s  login:%s\033[0m\n", username, date);
            break;
        }
        printf("\nlogin failed\n");
    }
#endif
    while(1)
    {
        i = 0;
        select = -1;
        memset(cmd,0,sizeof(cmd));
        printf("lshaluminum@lshaluminum:%s# ",CurentDir.curr_path);
        // scanf("%s",cmd);
        while (((c = getchar()) != '\n') && (i < 128)) {
            if (c == 32 || c == 9)
                cmd[i] = '\n';
            else
                cmd[i++] = c;
        }
        printf("%s",cmd);
        for (i=0;i<CommandNum;i++)
            if (!strcmp(cmd, command[i])) {
                select = i;
                break;
            }
        if (!strlen(cmd))
            select = 21;
        switch (select)
        {
            case 0:
                fmt_fs();
                break;
            case 1:
                exit_fs();
                goto end;
                break;
            case 2:
                scanf("%s",name);
                create_file(name, Dictionary);
                break;
            case 3:
                list_file();
                break;
            case 4:
                scanf("%s", name);
                change_dictionary(name);
                break;
            case 5:
                scanf("%s", name);
                remove_file(name, 1);
                break;
            case 6:
                scanf("%s", name);
                create_file(name, File);
                break;
            case 7:
                move_file();
                break;
            case 8:
                show_command();
                break;
            case 9:
                scanf("%s",name);
                read_file(name);
                break;
            case 10:
                show_map(BlkNum, super_blk.blk_map, 1);
                break;
            case 11:
                show_map(InodeNum, super_blk.inode_map, 0);
                break;
            case 12:
                scanf("%s", name);
                write_file(name);
                break;
            case 13:
                isLogin = 0;
                printf("bye %s\n", username);
                goto login;
                break;
            case 14:
                show_disk();
                break;
            case 15:
                //adduser();
                break;
            case 16:
                //chmod();
                break;
            case 17:
                //chown();
                break;
            case 18:
                //password();
                break;
            case 19:
                //ln();
                break;
            case 20:
                //userdel();
                break;
            case 21:
                break;
            default:
                printf("command not find\n");
                break;
        }
    }
end:
    fclose(Disk);
    return 0;
}

void show_map(int map_size, int* map, int type)
{
    int i;
    if (type == 1)
        printf("-------------------------------Block Map------------------------------------\n");
    else
        printf("-------------------------------Inode Map------------------------------------\n");
    printf("********************************************************************************\n");
    for (i=0;i<map_size;i++) {
        if (map[i])
            printf("%d ", map[i]);
        else
            printf("\033[1;32m%d\033[0m ",map[i]);
        if ((i+1)%40 == 0)
            printf("\n");
    }
    if (i%40 != 0)
        printf("\n");
    printf("********************************************************************************\n");
    printf("----------------------------------End Map---------------------------------------\n");
}
void set_console_color(unsigned short color_index)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_index);
}
void show_command()
{
    printf("------------------------------- List of Command ---------------------------------\n");
    for (int i=0;i<CommandNum;i++)
        printf("\033[1;32m%s\033[0m\n", command_help[i]);
    printf("------------------------------- End List ----------------------------------------\n");
}
int init_fs()//初始化文件系统
{
    fseek(Disk, SuperBlkBase, SEEK_SET);//偏移指针
    fread(&super_blk,sizeof(SuperBlk), 1, Disk);//读取数据
    CurentDir.inode_rank = 0;//设置根inode号
    strcpy(CurentDir.curr_path, "~");//设置当前路径
    if (open_dir(CurentDir.inode_rank) == -1)
        printf("open file system falied\n");
    return 0;
}
int fmt_fs()//格式化文件系统
{
    memset(super_blk.inode_map, 0, sizeof(super_blk.inode_map));
    memset(super_blk.blk_map, 0, sizeof(super_blk.blk_map));
    // fwrite("0",DiskSize,1,Disk);
    super_blk.inode_map[0] = 1;
    super_blk.inode_used = 1;
    super_blk.blk_map[0] = 1;
    super_blk.blk_used = 1;
    CurentDir.inode_rank = 0;
    CurentDir.dir_num = 2;
    strcpy(CurentDir.dir_item[0].filename, ".");
    CurentDir.dir_item[0].inode_rank = 0;
    strcpy(CurentDir.dir_item[1].filename, "..");
    CurentDir.dir_item[1].inode_rank = 0;
    CurentDir.curr_inode.file_size = CurentDir.dir_num * sizeof(FCB);
    CurentDir.curr_inode.file_blk_rank[0] = 0;
    CurentDir.curr_inode.blk_num = 1;
    strcpy(CurentDir.curr_inode.owner, username);
    strcpy(CurentDir.curr_inode.mode, "rwx");
    set_time(CurentDir.curr_inode.create_time);
    set_time(CurentDir.curr_inode.modified_time);
    CurentDir.curr_inode.type = Dictionary;
    rewrite_superblk();
    fseek(Disk, InodeBlkBase, SEEK_SET);
    fwrite(&CurentDir.curr_inode, sizeof(Inode), 1, Disk);
    fseek(Disk, DataBlkBase, SEEK_SET);
    fwrite(CurentDir.dir_item, CurentDir.curr_inode.file_size, 1, Disk);
    strcpy(CurentDir.curr_path, "~");
    printf("-------------------------------------------------------------\n");
    printf("\tformat filesystem complete!\n");
    printf("-------------------------------------------------------------\n");
    return 0;
}
void adduser(char* username)
{

}
void chmod(char* filename, int mode)
{

}
void chown(char* username)
{

}
void newpassword(char* password)
{

}
void ln(char* lname, char* filename)
{

}
void userdel(char* username)
{

}
void set_time(char* timec)
{
    time_t t = time(0);
    strftime(timec, 64, "%Y/%m/%d %X %A", localtime(&t));
}
int exit_fs()
{
    rewrite_superblk();
    return 0;
}
void rewrite_superblk()
{
    fseek(Disk, SuperBlkBase, SEEK_SET);
    fwrite(&super_blk, sizeof(SuperBlk), 1, Disk);
}
int list_file()//输出目录文件目录项
{
    for (int i=0;i<CurentDir.dir_num;i++) {
        if (type_check(CurentDir.dir_item[i].filename) == Dictionary)
            printf("%s\n",CurentDir.dir_item[i].filename);
        else if (type_check(CurentDir.dir_item[i].filename) == Link)
            ("%s\n", CurentDir.dir_item[i].filename);
        else
            printf("%s\n", CurentDir.dir_item[i].filename);
    }
    return 0;
}
int type_check(char* name)//判断文件类型
{
    int i, inode_num = -233;
    Inode temp;
    for (i=0;i<CurentDir.dir_num;i++) {
        if (!strcmp(CurentDir.dir_item[i].filename, name)) {//出现低级错误
            inode_num = CurentDir.dir_item[i].inode_rank;
            fseek(Disk,InodeBlkBase+inode_num*sizeof(Inode), SEEK_SET);
            fread(&temp, sizeof(Inode), 1, Disk);
            return temp.type;
        }
    }
}
int change_dictionary(char* name)//切换目录
{
    if (!strcmp(name, "."))
        return 0;
    if (check_rename(name)) {
        printf("Dictionary %s does not exist\n",name);
        return -1;
    }
    if (type_check(name) != Dictionary) {
        printf("cd %s failed, it is not a dictionary\n",name);
        return -1;
    }
    if (!strcmp(name, "..")) {
        int pos = strlen(CurentDir.curr_path)-1;
        for (;pos>=0;pos--) {
            if (CurentDir.curr_path[pos] == '/') {
                CurentDir.curr_path[pos] = '\0';
                break;
            }
        }
    }
    else {
        strcat(CurentDir.curr_path, "/");
        strcat(CurentDir.curr_path, name);
    }
    CurentDir.inode_rank = get_inode_rank(name);
    open_dir(CurentDir.inode_rank);
    return 0;
}
int get_inode_rank(char* name)//获取文件inode号
{
    int i;
    for (i=0;i<CurentDir.dir_num;i++)
        if (!strcmp(CurentDir.dir_item[i].filename, name))
            break;
    return CurentDir.dir_item[i].inode_rank;
}
//删除文件/文件夹,tag用来标志是否为要删除的根目录文件
int remove_file(char *name, int tag)
{
    if (check_rename(name)) {
        printf("file %s does not exist\n", name);
        return -1;
    }
    int inode_rank = get_inode_rank(name);
    if (type_check(name) == File) {
        free_file_blk(inode_rank);
        free_file_inode(inode_rank);
        //添加if判断，只有根路径才调整dir和重写supblk
        if (tag == 1) {
            adjust_dir(inode_rank);
            rewrite_superblk();
        }
    }
    else if (type_check(name) == Dictionary) {
        if (!is_empty_dictionary(inode_rank)) {
            if (tag == 1) {
                printf("%s is not a empty folder, still delete it? ", name);
                char c;
                scanf("%c", &c);
                scanf("%c", &c);
                if (c == 'n')
                return 0;
            }
            CurrentDir CurentDir1;
            //将当前目录拷贝下来
            // memcpy(&CurentDir1, &CurentDir, sizeof(CurentDir));
            change_dictionary(name);
            int i;
            for(i=2;i<CurentDir.dir_num;i++)
                remove_file(CurentDir.dir_item[i].filename, 0);
            // memcpy(&CurentDir, &CurentDir1, sizeof(CurentDir));
            free_file_blk(inode_rank);
            free_file_inode(inode_rank);
            char fatherinode[3] = "..";
            change_dictionary(fatherinode);
            //同上
            if (tag == 1) {
                adjust_dir(inode_rank);
                rewrite_superblk();
            }
        }
        else {
            free_file_blk(inode_rank);
            free_file_inode(inode_rank);
            if (tag == 1) {
                adjust_dir(inode_rank);
                rewrite_superblk();
            }
        }
    }
    return 0;
}
int is_empty_dictionary(int inode_rank) //判断是否为空文件夹
{
    Inode temp;
    fseek(Disk, InodeBlkBase+inode_rank*sizeof(Inode), SEEK_SET);
    fread(&temp, sizeof(Inode), 1, Disk);
    if (temp.file_size == (2*sizeof(FCB)))
        return 1;
    return 0;
}
void adjust_dir(int inode_rank)//删除文件夹后调整当前目录文件
{
    int i;
    for (i=0;i<CurentDir.dir_num;i++)
        if (CurentDir.dir_item[i].inode_rank == inode_rank)
            break;
    for (;i<CurentDir.dir_num;i++)
        CurentDir.dir_item[i] = CurentDir.dir_item[i+1];
    CurentDir.dir_num--;
    CurentDir.curr_inode.file_size -= sizeof(FCB);
    set_time(CurentDir.curr_inode.modified_time);
    rewrite_dir();
}
void free_file_inode(int inode_rank)//释放删除文件的inode
{
    super_blk.inode_map[inode_rank] = 0;
    super_blk.inode_used--;
}
void free_file_blk(int inode_rank)//释放删除文件的
{
    Inode temp;
    fseek(Disk, InodeBlkBase+inode_rank*sizeof(Inode), SEEK_SET);
    fread(&temp, sizeof(Inode), 1, Disk);
    for(int i=0;i<temp.blk_num;i++) {
        super_blk.blk_map[temp.file_blk_rank[i]] = 0;
        super_blk.blk_used--;
    }
}
int create_file(char* name, int type)//创建文件或者目录
{
    int new_inode_rank;
    if (!check_rename(name)) {
        printf("cannot create rename file, they have alredy existed!\n");
        return -1;
    }
    if (CurentDir.dir_num >= MaxItemPerDir) {
        printf("cannot create a file %s : no extra item for this dir\n", name);
        return -1;
    }
    if (!(new_inode_rank = apply_inode())) {
        printf("cannot create file, no extra inode be applied\n");
        return -1;
    }
    if (type == File)
        init_inode(new_inode_rank, File);
    else if (type == Dictionary)
        init_inode(new_inode_rank, Dictionary);
    strcpy(CurentDir.dir_item[CurentDir.dir_num].filename, name);
    CurentDir.dir_item[CurentDir.dir_num].inode_rank = new_inode_rank;
    CurentDir.dir_num++;
    CurentDir.curr_inode.file_size += sizeof(FCB);
    set_time(CurentDir.curr_inode.modified_time);
    rewrite_dir();//将当前目录写回磁盘
    rewrite_superblk();
    return 0;
}
void rewrite_dir()
{
    //这里居然也忘记*BlkSize 智障
    fseek(Disk,DataBlkBase+CurentDir.curr_inode.file_blk_rank[0]*BlkSize, SEEK_SET);
    fwrite(CurentDir.dir_item, CurentDir.curr_inode.file_size, 1, Disk);
    fseek(Disk,InodeBlkBase+CurentDir.inode_rank*sizeof(Inode),SEEK_SET);
    fwrite(&CurentDir.curr_inode, sizeof(Inode), 1, Disk);
}
void init_inode(int new_inode_rank, int type)//初始化inode并写回磁盘
{
    Inode new_inode;
    if (type == File) {
        new_inode.blk_num = 0;
        new_inode.file_size = 0;
        strcpy(new_inode.owner, username);
        strcpy(new_inode.mode, "rw");
        set_time(new_inode.create_time);
        set_time(new_inode.modified_time);
        new_inode.type = type;
        fseek(Disk,InodeBlkBase+new_inode_rank*sizeof(Inode), SEEK_SET);
        fwrite(&new_inode, sizeof(Inode), 1, Disk);
    }
    if (type == Dictionary) {
        int new_blk_rank;
        FCB tmp_FCB[2];
        if (!(new_blk_rank = apply_blk())) {
            printf("cannot create dic;no extra block\n");
            return;
        }
        new_inode.blk_num = 1;
        new_inode.file_blk_rank[0] = new_blk_rank;
        // printf("init_inode new_blk_rank %d\n", new_blk_rank);
        new_inode.file_size = 2*sizeof(FCB);
        strcpy(new_inode.owner, username);
        strcpy(new_inode.mode, "rwx");
        set_time(new_inode.create_time);
        set_time(new_inode.modified_time);
        new_inode.type = type;
        strcpy(tmp_FCB[0].filename, ".");
        tmp_FCB[0].inode_rank = new_inode_rank;
        // printf("init_inode new_inode_rank %d\n", new_inode_rank);
        strcpy(tmp_FCB[1].filename, "..");
        tmp_FCB[1].inode_rank = CurentDir.inode_rank;
        // printf(" init_inode CurentDir.inode_rank %d\n",  CurentDir.inode_rank);
        fseek(Disk,InodeBlkBase+new_inode_rank*sizeof(Inode), SEEK_SET);
        fwrite(&new_inode, sizeof(Inode), 1, Disk);
        fseek(Disk, DataBlkBase+new_blk_rank*BlkSize, SEEK_SET);
        fwrite(tmp_FCB, sizeof(FCB), 2, Disk);
    }
}
int apply_blk()
{
    int i;
    if (super_blk.blk_used >= BlkNum)
        return 0;
    for (i=0;i<BlkNum;i++) {
        if (!super_blk.blk_map[i]) {
            super_blk.blk_map[i] = 1;
            super_blk.blk_used++;
            return i;
        }
    }
}
int check_rename(char* name)
{
    int i;
    for (i=0;i<CurentDir.dir_num;i++)
        if (!strcmp(CurentDir.dir_item[i].filename, name))
            return 0;
    return 1;
}
int apply_inode()
{
    int i;
    if (super_blk.inode_used >= InodeNum)
        return 0;
    for (i=0;i<InodeNum;i++) {
        if (!super_blk.inode_map[i]) {
            super_blk.inode_map[i] = 1;
            super_blk.inode_used++;
            return i;
        }
    }
}
int open_dir(int inode_rank)//将inode节点的内容和文件内容读取到CurenDir中
{
    int i,  dir_num;
    // printf("open inode num::%d\n", inode_rank);
    fseek(Disk, InodeBlkBase+inode_rank*sizeof(Inode), SEEK_SET);
    fread(&CurentDir.curr_inode,  sizeof(Inode), 1, Disk);
    dir_num = (CurentDir.curr_inode.file_size/sizeof(FCB));
    // printf("open blk num::%d\n", CurentDir.curr_inode.file_blk_rank[0]);
    //害死我了，找了一整天居然是我忘记了*BlkSize
    fseek(Disk, DataBlkBase+CurentDir.curr_inode.file_blk_rank[0]*BlkSize, SEEK_SET);
    fread(CurentDir.dir_item,sizeof(FCB), dir_num, Disk);
    CurentDir.dir_num = dir_num;
    return 0;
}
int move_file()
{
    return 0;
}
int copy_file()
{
    return 0;
}
int read_file(char* name)
{
    if (check_rename(name)) {
        printf("%s does not exit!\n", name);
        return -1;
    }
    if (open_file(name) == -1) {
        printf("read failed, file %s is a empty file\n", name);
        return -1;
    }
    printf("%s\n",OpFile.buff);
    if (OpFile.buff)
        close_file();
    return 0;
}
int open_file(char* name)
{
    OpFile.inode_rank = get_inode_rank(name);
    fseek(Disk, InodeBlkBase+OpFile.inode_rank*sizeof(Inode), SEEK_SET);
    fread(&OpFile.inode, sizeof(Inode), 1, Disk);
    if (!OpFile.inode.file_size)
        return -1;
    OpFile.buff = (char*)malloc(OpFile.inode.file_size);
    memset(OpFile.buff,'\0', OpFile.inode.file_size);
    int i, left, pos = 0;
    for (i=0;i<OpFile.inode.blk_num-1;i++) {
        fseek(Disk, DataBlkBase+OpFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
        fread(OpFile.buff+pos, BlkSize, 1, Disk);
        pos += BlkSize;
    }
    left = OpFile.inode.file_size - i*BlkSize;
    fseek(Disk, DataBlkBase+OpFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
    fread(OpFile.buff+pos, left, 1, Disk);
    // printf("left==%d\n", left);
    return 0;
}
int close_file()
{
    free(OpFile.buff);
    return 0;
}
int write_file(char* name)
{
    if (check_rename(name)) {
        printf("cannot write  file %s does not exist\n", name);
        return -1;
    }
    if (type_check(name) == Dictionary) {
        printf("Cannot write a Dictionary\n");
        return -1;
    }
    char buff[MaxFileSize]="\0";
    int inode_rank = get_inode_rank(name);
    if (open_file(name) != -1)
        strcpy(buff, OpFile.buff);
    // printf("%s",buff);
    printf("---------- Write Start End With '\\' ---------\n");
    char c;
    int pos = strlen(buff);
    while ((c=getchar()) != '\\') {
        if (pos < MaxFileSize)
            buff[pos++] = c;
        else {
            printf("no space to write\n");
            break;
        }
    }
    printf("---------------- Write End -------------------\n");
    write_file_buff(buff);
    printf("File Saved\n");
    return 0;
}
int write_file_buff(char* buff)
{
    // printf("%s\n",buff);
    int buff_size = strlen(buff);
    OpFile.inode.file_size = buff_size;
    set_time(OpFile.inode.modified_time);
    int blk_num = buff_size/BlkSize + 1;
    int blk_need = blk_num - OpFile.inode.blk_num;
    // printf("%d\n", blk_need);
    int i, left, pos = 0;
    for (i=0;i<blk_need;i++) {
        OpFile.inode.file_blk_rank[OpFile.inode.blk_num++] = apply_blk();
    }
    for (i=0;i<OpFile.inode.blk_num-1;i++) {
        fseek(Disk, DataBlkBase+OpFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
        fwrite(buff+pos, BlkSize, 1, Disk);
        pos += BlkSize;
    }
    left = OpFile.inode.file_size - i*BlkSize;
    // printf("%d\n", left);
    fseek(Disk, DataBlkBase+OpFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
    fwrite(buff+pos, left, 1, Disk);
    fseek(Disk, InodeBlkBase+OpFile.inode_rank*sizeof(Inode), SEEK_SET);
    fwrite(&OpFile.inode, sizeof(Inode), 1, Disk);
}
int show_disk()
{
    int disk_size = DiskSize;
    int disk_used = super_blk.blk_used*BlkSize + super_blk.inode_used*sizeof(Inode) + sizeof(super_blk);
    int disk_free = disk_size - disk_used;
    printf("total\t\tused\t\tfree\t\n");
    printf("%d\t\t%d\t\t%d\n", disk_size, disk_used, disk_free);
    return 0;
}
///end
