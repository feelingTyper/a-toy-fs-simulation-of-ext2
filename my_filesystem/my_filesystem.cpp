#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include "fs.h"

//支持的操作，目前还有一些没有实现
const char* command[] = {"fmt","exit","mkdir","ls","cd","rm","touch","mv","help",
"read","blkmap","inodemap","write","logout","fdisk","adduser","chmod","chown","password","ln","userdel"};
const char* command_help[] =
{
    "help",
    "fmt",
    "exit",
    "ln [-s] [lnfilename] [filename]",
    "chmod [-R] [wrx] [filename]",
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
char username[32] = "lshaluminum";
char password[64] = "lshaluminum";
int isLogin = 0;
int main(void)
{
    int quit = 0;
    char name[32];
    char* p = NULL;
    int select = -1;
    char c, date[64];
    char scmd[10][48];
    char cmd[128] = "\0";
    int i = 0, tag = 0, j = 0;
    printf("%s------------------------------------------------------------%s\n",_YELLOW_,_END_);
    printf("%s\tFilsystem Version 1.0\n%s",_YELLOW_,_END_);
    printf("%s------------------------------------------------------------%s\n",_YELLOW_,_END_);
    Disk = fopen(DISK,"rb+");
    if (!Disk) {
        Disk = fopen(DISK,"wb+");
        fmt_fs();
    }
    init_fs();
login:
#ifndef TEST
    while (1) {
        i=0;
        c_printf("Username: ");
        while (((c = getchar()) != '\n') && (i < 32))
                username[i++] = c;
        c_printf("Password: ");
        system("stty -echo");
        i = 0;
        while (((c = getchar()) != '\n') && (i < 64))
                password[i++] = c;
        system("stty echo");
        if (!strcmp(username, RootUser) && !strcmp(password, PassWord)) {
            isLogin = 1;
            set_time(date);
            printf("\033[1;32m\nWelcome %s  login:%s\033[0m\n", username, date);
            break;
        }
        printf("%s\nlogin failed\n%s", _RED_, _END_);
    }
#endif
    while(1)
    {
        i = 0, tag = 0, j = 0;
        select = -1;
        memset(cmd, 0, sizeof(cmd));
        memset(scmd, 0, sizeof(scmd));
        printf("%slshaluminum@lshaluminum:%s# %s", _YELLOW_,cdir.curr_path,_END_);
        while ((c = getchar()) != '\n' && i < 128) {
            if (c == 32 || c == 9) {
                if(tag)
                    continue;
                cmd[i++] = 0;
                tag = 1;
            }
            else {
                cmd[i++] = c;
                tag = 0;
            }
        }
        i=0;
        while(i < 128) {
            p = scmd[j];
            while(cmd[i++] != '\0')
                memcpy(p++, &cmd[i-1], 1);
            if( p == scmd[j])
                break;
            j++;
        }
        if (!strlen(cmd))
            select = 21;
        else
            for (i=0;i<CommandNum;i++)
                if (!strcmp(scmd[0], command[i])) {
                    select = i;
                    break;
                }
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
                create_file(scmd[1], Dictionary);
                break;
            case 3:
                if(j == 1)
                    list_file();
                else if(j == 2)
                    if(!strcmp(scmd[1], "-l"))
                        long_list_file();
                    else 
                        list_file(scmd[1]);
                else if(j >= 3)
                    if(!strcmp(scmd[1], "-l"))
                        for(int k=2;k<j;k++)
                            long_list_file(scmd[k]);
                    else
                        for(int k=1;k<j;k++)
                            list_file(scmd[k]);
                break;
            case 4:
                change_dictionary(scmd[1]);
                break;
            case 5:
                for(int i=1;i<j;i++)
                    remove_file(scmd[i], 1);
                break;
            case 6:
                for(int i=1;i<j;i++)
                    create_file(scmd[i], File);
                break;
            case 7:
                move_file();
                break;
            case 8:
                show_command();
                break;
            case 9:
                read_file(scmd[1]);
                break;
            case 10:
                show_map(BlkNum, super_blk.blk_map, 1);
                break;
            case 11:
                show_map(InodeNum, super_blk.inode_map, 0);
                break;
            case 12:
                write_file(scmd[1]);
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
        c_printf("-------------------------------Block Map------------------------------------\n");
    else
        c_printf("-------------------------------Inode Map------------------------------------\n");
    c_printf("********************************************************************************\n");
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
    c_printf("********************************************************************************\n");
    c_printf("----------------------------------End Map---------------------------------------\n");
}
void c_printf(const char* name)
{
    printf("\033[1;32m%s\033[0m", name);
}
void c_printf(char* name)
{
    printf("\033[1;32m%s\033[0m", name);
}
void show_command()
{
    c_printf("------------------------------- List of Command ---------------------------------\n");
    for (int i=0;i<CommandNum;i++)
        printf("\033[1;32m%s\033[0m\n", command_help[i]);
    c_printf("------------------------------- End List ----------------------------------------\n");
}
int init_fs()//初始化文件系统
{
    char home[50];
    fseek(Disk, SuperBlkBase, SEEK_SET);//偏移指针
    fread(&super_blk, sizeof(SuperBlk), 1, Disk);//读取数据
    cdir.inode_rank = 0;//设置根inode号
    if (open_dir(cdir.inode_rank) == -1)
        printf("open file system falied\n");
    strcpy(cdir.curr_path, "/");//设置当前路径
    return 0;
}
int fmt_fs()//格式化文件系统
{
    char init_folder[7][48] = {
        "root",
        "etc",
        "bin",
        "home",
        "tmp",
        "var",
        "mnt",
    };
    memset(super_blk.inode_map, 0, sizeof(super_blk.inode_map));
    memset(super_blk.blk_map, 0, sizeof(super_blk.blk_map));
    super_blk.inode_map[0] = 1;
    super_blk.inode_used = 1;
    super_blk.blk_map[0] = 1;
    super_blk.blk_used = 1;
    cdir.inode_rank = 0;
    cdir.dir_num = 2;
    strcpy(cdir.dir_item[0].filename, ".");
    cdir.dir_item[0].inode_rank = 0;
    strcpy(cdir.dir_item[1].filename, "..");
    cdir.dir_item[1].inode_rank = 0;
    cdir.inode.file_size = cdir.dir_num * sizeof(FCB);
    cdir.inode.file_blk_rank[0] = 0;
    cdir.inode.blk_num = 1;
    strcpy(cdir.inode.owner, username);
    strcpy(cdir.inode.mode, "rwx");
    set_time(cdir.inode.create_time);
    set_time(cdir.inode.modified_time);
    cdir.inode.type = Dictionary;
    rewrite_superblk();
    fseek(Disk, InodeBlkBase, SEEK_SET);
    fwrite(&cdir.inode, sizeof(Inode), 1, Disk);
    fseek(Disk, DataBlkBase, SEEK_SET);
    fwrite(cdir.dir_item, cdir.inode.file_size, 1, Disk);
    for(int i=0;i<7;i++)
        create_file(init_folder[i], Dictionary);
    strcpy(cdir.curr_path, "/");
    printf("%s-------------------------------------------------------------%s\n",_RED_,_END_);
    printf("%s\tformat filesystem complete!%s\n", _RED_, _END_);
    printf("%s-------------------------------------------------------------%s\n",_RED_,_END_);
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
    int i;
    for (i=2;i < cdir.dir_num;i++) {
        if (type_check(cdir.dir_item[i].filename) == Dictionary)
            printf("%s%s    %s", _GREEN_,cdir.dir_item[i].filename,_END_);
        else if (type_check(cdir.dir_item[i].filename) == Link)
            printf("%s%s    %s", _YELLOW_,cdir.dir_item[i].filename,_END_);
        else
            printf("%s    ", cdir.dir_item[i].filename);
        if(i%8 == 0)
            printf("\n");
    }
    if((i-1)%8)
        printf("\n");
    return 0;
}
int long_list_file()
{
    Inode temp_inode;
    int inode_rank;
    for (int i=2;i<cdir.dir_num;i++) {
        inode_rank = get_inode_rank(cdir.dir_item[i].filename);
        fseek(Disk, InodeBlkBase + sizeof(Inode)*inode_rank, SEEK_SET);
        fread(&temp_inode, sizeof(Inode), 1, Disk);
        if (temp_inode.type == Dictionary)
            printf("%sd%s %s %d %s  %s %s\n", _GREEN_, temp_inode.mode, temp_inode.owner, temp_inode.file_size, temp_inode.create_time, cdir.dir_item[i].filename, _END_);
        else
            printf("-%s %s %d %s  %s\n", temp_inode.mode, temp_inode.owner, temp_inode.file_size, temp_inode.create_time, cdir.dir_item[i].filename);
    }
    return 0;
}
int long_list_file(char* name)
{
    Inode temp_inode;
    if (check_rename(name)) {
        printf("ls: cannot access '%s': No such file or directory\n", name);
        return -1;
    }
    int inode_rank = get_inode_rank(name);
    fseek(Disk, InodeBlkBase + sizeof(Inode)*inode_rank, SEEK_SET);
    fread(&temp_inode, sizeof(Inode), 1, Disk);
    if (temp_inode.type == Dictionary)
        printf("%sd%s %s %d %s  %s %s\n", _GREEN_, temp_inode.mode, temp_inode.owner, temp_inode.file_size, temp_inode.create_time, name, _END_);
    else
        printf("-%s %s %d %s  %s\n", temp_inode.mode, temp_inode.owner, temp_inode.file_size, temp_inode.create_time, name);
	return 0;
}

int list_file(char* name)
{
    if (check_rename(name)) {
        printf("ls: cannot access '%s': No such file or directory\n", name);
        return -1;
    }
    if (type_check(name) == Dictionary)
        printf("%s%s%s\n", _GREEN_, name, _END_);
    else
        printf("%s\n", name);
    return 0;
}
int type_check(char* name)//判断文件类型
{
    int i, inode_num = -233;
    Inode temp;
    for (i=0;i<cdir.dir_num;i++) {
        if (!strcmp(cdir.dir_item[i].filename, name)) {//出现低级错误
            inode_num = cdir.dir_item[i].inode_rank;
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
        printf("%sDictionary %s does not exist%s\n", _RED_,name,_END_);
        return -1;
    }
    if (type_check(name) != Dictionary) {
        printf("%scd %s failed, it is not a dictionary%s\n", _RED_,name,_END_);
        return -1;
    }
    if (!strcmp(name, "..")) {
        int pos = strlen(cdir.curr_path)-1;
        for (;pos>=0;pos--) {
            if (cdir.curr_path[pos] == '/') {
                cdir.curr_path[pos] = '\0';
                break;
            }
        }
    }
    else {
        strcat(cdir.curr_path, "/");
        strcat(cdir.curr_path, name);
    }
    cdir.inode_rank = get_inode_rank(name);
    open_dir(cdir.inode_rank);
	return 0;
}
int get_inode_rank(char* name)//获取文件inode号
{
    int i;
    for (i=0;i<cdir.dir_num;i++)
        if (!strcmp(cdir.dir_item[i].filename, name))
            break;
    return cdir.dir_item[i].inode_rank;
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
        if (tag == 1) { //cannot delete this judge
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
            change_dictionary(name);
            int i;
            for(i=2;i<cdir.dir_num;i++)
                remove_file(cdir.dir_item[i].filename, 0);
            free_file_blk(inode_rank);
            free_file_inode(inode_rank);
            char fatherinode[3] = "..";
            change_dictionary(fatherinode);
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
    for (i=0;i<cdir.dir_num;i++)
        if (cdir.dir_item[i].inode_rank == inode_rank)
            break;
    for (;i<cdir.dir_num;i++)
        cdir.dir_item[i] = cdir.dir_item[i+1];
    cdir.dir_num--;
    cdir.inode.file_size -= sizeof(FCB);
    set_time(cdir.inode.modified_time);
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
    if (cdir.dir_num >= MaxItemPerDir) {
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
    strcpy(cdir.dir_item[cdir.dir_num].filename, name);
    cdir.dir_item[cdir.dir_num].inode_rank = new_inode_rank;
    cdir.dir_num++;
    cdir.inode.file_size += sizeof(FCB);
    set_time(cdir.inode.modified_time);
    rewrite_dir();//将当前目录写回磁盘
    rewrite_superblk();
	return 0;
}
void rewrite_dir()
{
    //这里居然也忘记*BlkSize 智障
    fseek(Disk,DataBlkBase+cdir.inode.file_blk_rank[0]*BlkSize, SEEK_SET);
    fwrite(cdir.dir_item, cdir.inode.file_size, 1, Disk);
    fseek(Disk,InodeBlkBase+cdir.inode_rank*sizeof(Inode),SEEK_SET);
    fwrite(&cdir.inode, sizeof(Inode), 1, Disk);
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
        strcpy(tmp_FCB[1].filename, "..");
        tmp_FCB[1].inode_rank = cdir.inode_rank;
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
    for (i=0;i<cdir.dir_num;i++)
        if (!strcmp(cdir.dir_item[i].filename, name))
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
    fseek(Disk, InodeBlkBase+inode_rank*sizeof(Inode), SEEK_SET);
    fread(&cdir.inode,  sizeof(Inode), 1, Disk);
    dir_num = (cdir.inode.file_size/sizeof(FCB));
    //md找了一整天居然是忘记了*BlkSize
    fseek(Disk, DataBlkBase+cdir.inode.file_blk_rank[0]*BlkSize, SEEK_SET);
    fread(cdir.dir_item,sizeof(FCB), dir_num, Disk);
    cdir.dir_num = dir_num;
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
    printf("%s\n",OpenFile.buff);
    if (OpenFile.buff)
        close_file();
    return 0;
}
int open_file(char* name)
{
    OpenFile.inode_rank = get_inode_rank(name);
    fseek(Disk, InodeBlkBase+OpenFile.inode_rank*sizeof(Inode), SEEK_SET);
    fread(&OpenFile.inode, sizeof(Inode), 1, Disk);
    if (!OpenFile.inode.file_size)
        return -1;
    OpenFile.buff = (char*)malloc(OpenFile.inode.file_size);
    memset(OpenFile.buff,'\0', OpenFile.inode.file_size);
    int i, left, pos = 0;
    for (i=0;i<OpenFile.inode.blk_num-1;i++) {
        fseek(Disk, DataBlkBase+OpenFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
        fread(OpenFile.buff+pos, BlkSize, 1, Disk);
        pos += BlkSize;
    }
    left = OpenFile.inode.file_size - i*BlkSize;
    fseek(Disk, DataBlkBase+OpenFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
    fread(OpenFile.buff+pos, left, 1, Disk);
    return 0;
}
int close_file()
{
    free(OpenFile.buff);
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
        strcpy(buff, OpenFile.buff);
    printf("\033[1;32m---------- Write Start End With '*END*' ---------\033[0m\n");
    char c;
    char* tmp;
    int pos = strlen(buff);
    while (c = getchar() && pos < MaxFileSize) {
            buff[pos++] = c;
            if (tmp = strstr(&buff[pos-9],"*END*")) {
                *tmp = '\0';
                break;
            }
    }
    printf("\033[1;32m---------------- Write End -------------------\033[0m\n");
    write_file_buff(buff);
    printf("File Saved\n");
	return 0;
}
int write_file_buff(char* buff)
{
    printf("%s\n",buff);
    int buff_size = strlen(buff);
    OpenFile.inode.file_size = buff_size;
    set_time(OpenFile.inode.modified_time);
    int blk_num = buff_size/BlkSize + 1;
    int blk_need = blk_num - OpenFile.inode.blk_num;
    int i, left, pos = 0;
    for (i=0;i<blk_need;i++) {
        OpenFile.inode.file_blk_rank[OpenFile.inode.blk_num++] = apply_blk();
    }
    for (i=0;i<OpenFile.inode.blk_num-1;i++) {
        fseek(Disk, DataBlkBase+OpenFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
        fwrite(buff+pos, BlkSize, 1, Disk);
        pos += BlkSize;
    }
    left = OpenFile.inode.file_size - i*BlkSize;
    fseek(Disk, DataBlkBase+OpenFile.inode.file_blk_rank[i]*BlkSize, SEEK_SET);
    fwrite(buff+pos, left, 1, Disk);
    fseek(Disk, InodeBlkBase+OpenFile.inode_rank*sizeof(Inode), SEEK_SET);
    fwrite(&OpenFile.inode, sizeof(Inode), 1, Disk);
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
