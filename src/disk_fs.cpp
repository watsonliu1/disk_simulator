#include "../include/disk_fs.h"
#include <cstring>
#include <iostream>
#include <ctime>
#include <iomanip>

DiskFS::DiskFS(const std::string& path) : disk_path(path), is_mounted(false) {}

DiskFS::~DiskFS() {
    if (is_mounted) {
        unmount();
    }
}

uint32_t DiskFS::get_inode_pos(uint32_t inode_num) {
    if (inode_num >= super_block.total_inodes) return 0;
    return super_block.inode_start * BLOCK_SIZE + inode_num * INODE_SIZE;
}

uint32_t DiskFS::get_data_block_pos(uint32_t block_num) {
    if (block_num >= super_block.total_blocks) return 0;
    return super_block.data_start * BLOCK_SIZE + (block_num - super_block.data_start) * BLOCK_SIZE;
}

bool DiskFS::set_block_bitmap(uint32_t block_num, bool used) {
    if (block_num < super_block.data_start || block_num >= super_block.total_blocks)
        return false;

    char buffer[BLOCK_SIZE];
    uint32_t bitmap_block = super_block.block_bitmap;
    if (!read_block(bitmap_block, buffer)) return false;

    uint32_t idx = block_num - super_block.data_start;
    uint32_t byte = idx / 8;
    uint8_t bit = idx % 8;

    if (used) {
        buffer[byte] |= (1 << bit);
        super_block.free_blocks--;
    } else {
        buffer[byte] &= ~(1 << bit);
        super_block.free_blocks++;
    }

    if (!write_block(bitmap_block, buffer)) return false;
    return true;
}

bool DiskFS::set_inode_bitmap(uint32_t inode_num, bool used) {
    if (inode_num >= super_block.total_inodes) return false;

    char buffer[BLOCK_SIZE];
    uint32_t bitmap_block = super_block.inode_bitmap;
    if (!read_block(bitmap_block, buffer)) return false;

    uint32_t byte = inode_num / 8;
    uint8_t bit = inode_num % 8;

    if (used) {
        buffer[byte] |= (1 << bit);
        super_block.free_inodes--;
    } else {
        buffer[byte] &= ~(1 << bit);
        super_block.free_inodes++;
    }

    if (!write_block(bitmap_block, buffer)) return false;
    return true;
}

int DiskFS::find_free_block() {
    char buffer[BLOCK_SIZE];
    if (!read_block(super_block.block_bitmap, buffer)) return -1;

    uint32_t total_data_blocks = super_block.data_blocks;
    
    for (uint32_t i = 0; i < total_data_blocks; i++) {
        uint32_t byte = i / 8;
        uint8_t bit = i % 8;
        
        if (!(buffer[byte] & (1 << bit))) {
            return super_block.data_start + i;
        }
    }
    return -1;
}

int DiskFS::find_free_inode() {
    char buffer[BLOCK_SIZE];
    if (!read_block(super_block.inode_bitmap, buffer)) return -1;

    for (uint32_t i = 0; i < super_block.total_inodes; i++) {
        uint32_t byte = i / 8;
        uint8_t bit = i % 8;
        
        if (!(buffer[byte] & (1 << bit))) {
            return i;
        }
    }
    return -1;
}

bool DiskFS::read_block(uint32_t block_num, char* buffer) {
    if (block_num >= super_block.total_blocks) return false;
    
    uint32_t pos = block_num * BLOCK_SIZE;
    disk_file.seekg(pos);
    disk_file.read(buffer, BLOCK_SIZE);
    return disk_file.good();
}

bool DiskFS::write_block(uint32_t block_num, const char* buffer) {
    if (block_num >= super_block.total_blocks) return false;
    
    uint32_t pos = block_num * BLOCK_SIZE;
    disk_file.seekp(pos);
    disk_file.write(buffer, BLOCK_SIZE);
    return disk_file.good();
}

bool DiskFS::format() {
    // 打开文件并设置大小
    disk_file.open(disk_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!disk_file) {
        disk_file.open(disk_path, std::ios::trunc | std::ios::out | std::ios::in | std::ios::binary);
        if (!disk_file) return false;
    }

    // 计算各区域大小
    uint32_t super_block_size = 1;  // 超级块占用1个块
    uint32_t block_bitmap_size = (MAX_BLOCKS + 7) / 8 / BLOCK_SIZE + 1;
    uint32_t inode_bitmap_size = (MAX_INODES + 7) / 8 / BLOCK_SIZE + 1;
    uint32_t inode_area_size = (MAX_INODES * INODE_SIZE + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // 初始化超级块
    memset(&super_block, 0, sizeof(SuperBlock));
    strcpy(super_block.magic, "SIMFSv1");
    super_block.block_size = BLOCK_SIZE;
    super_block.total_blocks = MAX_BLOCKS;
    super_block.inode_blocks = inode_area_size;
    super_block.data_blocks = MAX_BLOCKS - (super_block_size + block_bitmap_size + inode_bitmap_size + inode_area_size);
    super_block.total_inodes = MAX_INODES;
    super_block.free_blocks = super_block.data_blocks;
    super_block.free_inodes = MAX_INODES - 1;  // 预留根目录inode
    
    // 设置各区域起始块号
    super_block.block_bitmap = super_block_size;
    super_block.inode_bitmap = super_block.block_bitmap + block_bitmap_size;
    super_block.inode_start = super_block.inode_bitmap + inode_bitmap_size;
    super_block.data_start = super_block.inode_start + inode_area_size;

    // 写入超级块
    disk_file.seekp(0);
    disk_file.write((char*)&super_block, sizeof(SuperBlock));

    // 初始化块位图 (全部设为0，表示空闲)
    char buffer[BLOCK_SIZE] = {0};
    for (uint32_t i = 0; i < block_bitmap_size; i++) {
        write_block(super_block.block_bitmap + i, buffer);
    }

    // 初始化inode位图 (除根目录外全部设为0)
    memset(buffer, 0, BLOCK_SIZE);
    for (uint32_t i = 0; i < inode_bitmap_size; i++) {
        write_block(super_block.inode_bitmap + i, buffer);
    }

    // 标记根目录inode为已使用
    set_inode_bitmap(0, true);

    // 初始化inode区
    Inode inode;
    memset(&inode, 0, sizeof(Inode));
    for (uint32_t i = 0; i < MAX_INODES; i++) {
        inode.inode_num = i;
        inode.used = 0;
        disk_file.seekp(get_inode_pos(i));
        disk_file.write((char*)&inode, sizeof(Inode));
    }

    // 初始化根目录inode (inode 0)
    time_t now = time(nullptr);
    Inode root_inode;
    memset(&root_inode, 0, sizeof(Inode));
    root_inode.inode_num = 0;
    root_inode.type = 2;  // 目录类型
    root_inode.used = 1;
    root_inode.create_time = now;
    root_inode.modify_time = now;
    
    // 分配一个数据块给根目录
    int root_block = find_free_block();
    if (root_block != -1) {
        root_inode.blocks[0] = root_block;
        root_inode.size = BLOCK_SIZE;
        set_block_bitmap(root_block, true);
        
        // 初始化根目录内容
        memset(buffer, 0, BLOCK_SIZE);
        DirEntry* root_entry = (DirEntry*)buffer;
        strcpy(root_entry[0].name, ".");    // 当前目录
        root_entry[0].inode_num = 0;
        root_entry[0].valid = 1;
        write_block(root_block, buffer);
    }

    disk_file.seekp(get_inode_pos(0));
    disk_file.write((char*)&root_inode, sizeof(Inode));

    disk_file.close();
    return true;
}

bool DiskFS::mount() {
    if (is_mounted) return true;

    disk_file.open(disk_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!disk_file) return false;

    // 读取超级块
    disk_file.seekg(0);
    disk_file.read((char*)&super_block, sizeof(SuperBlock));

    // 验证文件系统标识
    if (strncmp(super_block.magic, "SIMFSv1", 7) != 0) {
        disk_file.close();
        return false;
    }

    is_mounted = true;
    return true;
}

bool DiskFS::unmount() {
    if (!is_mounted) return true;

    // 写回超级块
    disk_file.seekp(0);
    disk_file.write((char*)&super_block, sizeof(SuperBlock));
    
    disk_file.close();
    is_mounted = false;
    return true;
}

int DiskFS::create_file(const std::string& name) {
    if (!isMounted() || name.length() >= MAX_FILENAME) return -1;

    // 检查文件是否已存在（修改变量名避免冲突）
    std::vector<DirEntry> dir_list = list_files();
    for (const auto& entry : dir_list) {
        if (entry.valid && name == entry.name) {
            return -1;  // 文件已存在
        }
    }

    // 获取根目录inode
    Inode root_inode;
    disk_file.seekg(get_inode_pos(0));
    disk_file.read((char*)&root_inode, sizeof(Inode));
    if (root_inode.type != 2) return -1;  // 不是目录

    // 分配新inode
    int inode_num = find_free_inode();
    if (inode_num == -1) return -1;

    // 初始化新inode
    time_t now = time(nullptr);
    Inode new_inode;
    memset(&new_inode, 0, sizeof(Inode));
    new_inode.inode_num = inode_num;
    new_inode.type = 1;  // 文件类型
    new_inode.used = 1;
    new_inode.create_time = now;
    new_inode.modify_time = now;

    disk_file.seekp(get_inode_pos(inode_num));
    disk_file.write((char*)&new_inode, sizeof(Inode));
    set_inode_bitmap(inode_num, true);

    // 在根目录中添加条目（修改变量名和循环变量类型）
    char buffer[BLOCK_SIZE];
    if (!read_block(root_inode.blocks[0], buffer)) return -1;

    DirEntry* dir_entries = (DirEntry*)buffer;
    for (size_t i = 1; i < BLOCK_SIZE / sizeof(DirEntry); i++) {  // 用size_t避免符号冲突
        if (!dir_entries[i].valid) {
            strncpy(dir_entries[i].name, name.c_str(), MAX_FILENAME - 1);
            dir_entries[i].inode_num = inode_num;
            dir_entries[i].valid = 1;
            break;
        }
    }

    write_block(root_inode.blocks[0], buffer);

    // 更新根目录修改时间
    root_inode.modify_time = now;
    disk_file.seekp(get_inode_pos(0));
    disk_file.write((char*)&root_inode, sizeof(Inode));

    return inode_num;
}

int DiskFS::open_file(const std::string& name) {
    if (!isMounted()) return -1;

    std::vector<DirEntry> entries = list_files();
    for (const auto& entry : entries) {
        if (entry.valid && name == entry.name) {
            return entry.inode_num;
        }
    }
    return -1;  // 文件不存在
}

int DiskFS::read_file(int inode_num, char* buffer, size_t size, off_t offset) {
    // 修正inode_num类型比较
    if (!isMounted() || inode_num < 0 || (uint32_t)inode_num >= super_block.total_inodes) 
        return -1;

    // 读取inode
    Inode inode;
    disk_file.seekg(get_inode_pos(inode_num));
    disk_file.read((char*)&inode, sizeof(Inode));
    if (!inode.used || inode.type != 1) return -1;  // 不是有效的文件

    // 计算可读取的字节数（移除无效的<0判断）
    size_t read_size = size;
    if (offset + read_size > inode.size) {
        read_size = inode.size - offset;
    }

    if (read_size == 0) return 0;

    char block_buffer[BLOCK_SIZE];
    size_t bytes_read = 0;
    off_t current_offset = offset;

    // 读取数据块
    while (bytes_read < read_size) {
        uint32_t block_idx = current_offset / BLOCK_SIZE;
        if (block_idx >= 16) break;  // 超过直接块数量

        uint32_t block_num = inode.blocks[block_idx];
        if (block_num == 0) break;  // 该块未分配

        if (!read_block(block_num, block_buffer)) return -1;

        off_t block_offset = current_offset % BLOCK_SIZE;
        size_t to_read = std::min((size_t)(BLOCK_SIZE - block_offset), read_size - bytes_read);

        memcpy(buffer + bytes_read, block_buffer + block_offset, to_read);

        bytes_read += to_read;
        current_offset += to_read;
    }

    return bytes_read;
}

int DiskFS::write_file(int inode_num, const char* buffer, size_t size, off_t offset) {
    // 修正inode_num类型比较
    if (!isMounted() || inode_num < 0 || (uint32_t)inode_num >= super_block.total_inodes || !buffer) 
        return -1;

    // 读取inode
    Inode inode;
    disk_file.seekg(get_inode_pos(inode_num));
    disk_file.read((char*)&inode, sizeof(Inode));
    if (!inode.used || inode.type != 1) return -1;  // 不是有效的文件

    char block_buffer[BLOCK_SIZE];
    size_t bytes_written = 0;
    off_t current_offset = offset;
    time_t now = time(nullptr);

    // 写入数据块（修正块号判断）
    while (bytes_written < size) {
        uint32_t block_idx = current_offset / BLOCK_SIZE;
        if (block_idx >= 16) break;  // 超过直接块数量

        uint32_t block_num = inode.blocks[block_idx];
        // 先用int接收块号，避免无符号与-1比较
        if (block_num == 0) {
            int free_block = find_free_block();
            if (free_block == -1) break;  // 没有空闲块
            block_num = free_block;

            inode.blocks[block_idx] = block_num;
            set_block_bitmap(block_num, true);
            memset(block_buffer, 0, BLOCK_SIZE);
        } else {
            if (!read_block(block_num, block_buffer)) return -1;
        }

        off_t block_offset = current_offset % BLOCK_SIZE;
        size_t to_write = std::min((size_t)(BLOCK_SIZE - block_offset), size - bytes_written);

        memcpy(block_buffer + block_offset, buffer + bytes_written, to_write);
        if (!write_block(block_num, block_buffer)) return -1;

        bytes_written += to_write;
        current_offset += to_write;
    }

    // 更新文件大小和修改时间
    if (current_offset > inode.size) {
        inode.size = current_offset;
    }
    inode.modify_time = now;

    disk_file.seekp(get_inode_pos(inode_num));
    disk_file.write((char*)&inode, sizeof(Inode));

    return bytes_written;
}

bool DiskFS::delete_file(const std::string& name) {
    if (!isMounted()) return false;

    // 获取根目录inode和内容
    Inode root_inode;
    disk_file.seekg(get_inode_pos(0));
    disk_file.read((char*)&root_inode, sizeof(Inode));
    if (root_inode.type != 2) return false;

    char buffer[BLOCK_SIZE];
    if (!read_block(root_inode.blocks[0], buffer)) return false;

    DirEntry* entries = (DirEntry*)buffer;
    int target_inode = -1;

    // 修正循环变量类型
    for (size_t i = 1; i < BLOCK_SIZE / sizeof(DirEntry); i++) {
        if (entries[i].valid && name == entries[i].name) {
            target_inode = entries[i].inode_num;
            entries[i].valid = 0;  // 标记为无效
            break;
        }
    }

    if (target_inode == -1) return false;

    // 更新根目录
    write_block(root_inode.blocks[0], buffer);
    root_inode.modify_time = time(nullptr);
    disk_file.seekp(get_inode_pos(0));
    disk_file.write((char*)&root_inode, sizeof(Inode));

    // 回收文件inode和数据块
    Inode file_inode;
    disk_file.seekg(get_inode_pos(target_inode));
    disk_file.read((char*)&file_inode, sizeof(Inode));

    // 释放数据块
    for (int i = 0; i < 16; i++) {
        if (file_inode.blocks[i] != 0) {
            set_block_bitmap(file_inode.blocks[i], false);
            file_inode.blocks[i] = 0;
        }
    }

    // 释放inode
    file_inode.used = 0;
    disk_file.seekp(get_inode_pos(target_inode));
    disk_file.write((char*)&file_inode, sizeof(Inode));
    set_inode_bitmap(target_inode, false);

    return true;
}

std::vector<DirEntry> DiskFS::list_files() {
    std::vector<DirEntry> result;
    if (!isMounted()) return result;

    // 读取根目录
    Inode root_inode;
    disk_file.seekg(get_inode_pos(0));
    disk_file.read((char*)&root_inode, sizeof(Inode));
    if (root_inode.type != 2) return result;

    char buffer[BLOCK_SIZE];
    if (!read_block(root_inode.blocks[0], buffer)) return result;

    DirEntry* entries = (DirEntry*)buffer;
    // 修正循环变量类型
    for (size_t i = 0; i < BLOCK_SIZE / sizeof(DirEntry); i++) {
        if (entries[i].valid) {
            result.push_back(entries[i]);
        }
    }

    return result;
}

void DiskFS::print_info() {
    if (!isMounted()) return;

    std::cout << "磁盘文件系统信息:\n";
    std::cout << "-------------------\n";
    std::cout << "文件系统: " << super_block.magic << "\n";
    std::cout << "块大小: " << super_block.block_size << " 字节\n";
    std::cout << "总块数: " << super_block.total_blocks << "\n";
    std::cout << "数据块数: " << super_block.data_blocks << "\n";
    std::cout << "总inode数: " << super_block.total_inodes << "\n";
    std::cout << "已使用空间: " << (super_block.data_blocks - super_block.free_blocks) * BLOCK_SIZE / 1024 << " KB\n";
    std::cout << "空闲空间: " << super_block.free_blocks * BLOCK_SIZE / 1024 << " KB\n";
    std::cout << "总大小: " << super_block.total_blocks * BLOCK_SIZE / 1024 << " KB\n";
}