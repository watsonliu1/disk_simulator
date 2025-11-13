#ifndef DISK_FS_H
#define DISK_FS_H

#include <string>
#include <cstdint>
#include <fstream>
#include <vector>

// 常量定义
const int BLOCK_SIZE = 4096;               // 块大小4KB
const int INODE_SIZE = 128;                // inode大小
const int MAX_FILENAME = 28;               // 最大文件名长度
const int MAX_INODES = 1024;               // 最大inode数量
const int MAX_BLOCKS = (1024 * 1024 * 100) / BLOCK_SIZE;  // 100MB磁盘

// inode结构
struct Inode {
    uint32_t inode_num;      // inode编号
    uint32_t size;           // 文件大小
    uint32_t blocks[16];     // 数据块指针 (直接块)
    uint8_t type;            // 文件类型 (1:文件, 2:目录)
    uint8_t used;            // 是否使用 (1:使用中, 0:未使用)
    time_t create_time;      // 创建时间
    time_t modify_time;      // 修改时间
};

// 目录项结构
struct DirEntry {
    char name[MAX_FILENAME]; // 文件名
    uint32_t inode_num;      // 对应的inode编号
    uint8_t valid;           // 是否有效
};

// 超级块结构
struct SuperBlock {
    char magic[8];           // 文件系统标识
    uint32_t block_size;     // 块大小
    uint32_t total_blocks;   // 总块数
    uint32_t inode_blocks;   // inode区占用块数
    uint32_t data_blocks;    // 数据区占用块数
    uint32_t total_inodes;   // 总inode数
    uint32_t free_blocks;    // 空闲块数
    uint32_t free_inodes;    // 空闲inode数
    uint32_t block_bitmap;   // 块位图起始块号
    uint32_t inode_bitmap;   // inode位图起始块号
    uint32_t inode_start;    // inode区起始块号
    uint32_t data_start;     // 数据区起始块号
};

class DiskFS {
private:
    std::fstream disk_file;
    std::string disk_path;
    SuperBlock super_block;
    bool is_mounted;

    // 计算各区域位置
    uint32_t get_super_block_pos() { return 0; }
    uint32_t get_block_bitmap_pos() { return super_block.block_bitmap * BLOCK_SIZE; }
    uint32_t get_inode_bitmap_pos() { return super_block.inode_bitmap * BLOCK_SIZE; }
    uint32_t get_inode_pos(uint32_t inode_num);
    uint32_t get_data_block_pos(uint32_t block_num);

    // 位图操作
    bool set_block_bitmap(uint32_t block_num, bool used);
    bool set_inode_bitmap(uint32_t inode_num, bool used);
    int find_free_block();
    int find_free_inode();

    // 读取/写入块
    bool read_block(uint32_t block_num, char* buffer);
    bool write_block(uint32_t block_num, const char* buffer);

public:
    DiskFS(const std::string& path);
    ~DiskFS();

    // 磁盘操作
    bool format();
    bool mount();
    bool unmount();

    // 文件操作
    int create_file(const std::string& name);
    int open_file(const std::string& name);
    int read_file(int inode_num, char* buffer, size_t size, off_t offset);
    int write_file(int inode_num, const char* buffer, size_t size, off_t offset);
    bool delete_file(const std::string& name);
    std::vector<DirEntry> list_files();

    // 信息查询
    void print_info();
    bool isMounted() const { return is_mounted; }  // 修正函数名
};

#endif // DISK_FS_H