#define TEST 100
#define BlkSize 1024
#define InodeNum 512
#define BlkNum 1024
#define MaxBlkPerFile 16
#define MaxFileSize (MaxBlkPerFile*BlkSize) //最大文件8kb
#define BuffSize MaxFileSize
#define DiskSize (BlkSize*BlkNum + InodeNum*sizeof(Inode) + sizeof(SuperBlk))//1177608
#define RootUser "test"
#define PassWord "test"
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
#define _RED_ "\033[1;31m"
#define _GREEN_ "\033[1;32m"
#define _YELLOW_ "\033[1;33m"
#define _BLUE_ "\033[1;34m"
#define _END_ "\033[0m"


typedef struct
{
    int blk_used;//block使用统计
    int inode_used;//inode使用统计
    int blk_map[BlkNum];//block使用情况位图
    int inode_map[InodeNum];//inode使用位图
} SuperBlk;

typedef struct
{
    int type;//文件类型
    int blk_num ;//文件占用数据块数量
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
    Inode inode;//当前目录文件的inode的内容
    char curr_path[128];//当前的路径
    FCB dir_item[MaxItemPerDir];//当前目录文件的内容
} cdir;

struct OpenFile //当前打开的普通文件
{
    int inode_rank;//当前打开文件的inode节点号
    Inode inode;//当前打开文件的详细信息，即其对应的inode的内容
    char* buff;//缓冲区
} OpenFile = {0};//初始化为0，防止出现野指针
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
int long_list_file();
int list_file(char*);
int long_list_file(char*);
