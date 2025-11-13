#include "include/disk_fs.h"
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief 打印帮助信息，展示所有支持的命令及用法
 */
void print_help() {
    std::cout << "磁盘模拟文件系统命令:\n";
    std::cout << "  format      - 格式化磁盘（初始化文件系统结构）\n";
    std::cout << "  mount       - 挂载磁盘（加载文件系统到内存）\n";
    std::cout << "  umount      - 卸载磁盘（将内存数据写回磁盘并关闭）\n";
    std::cout << "  info        - 显示磁盘信息（总容量、空闲空间等）\n";
    std::cout << "  create <文件名> - 创建文件（返回inode编号）\n";
    std::cout << "  open <文件名>   - 打开文件（获取文件对应的inode编号）\n";
    std::cout << "  read <inode> <大小> - 读取文件内容（从指定inode读取指定字节数）\n";
    std::cout << "  write <inode> <内容> - 写入文件内容（向指定inode写入内容）\n";
    std::cout << "  delete <文件名> - 删除文件（释放inode和数据块）\n";
    std::cout << "  ls          - 列出所有文件（显示文件名及对应inode）\n";
    std::cout << "  help        - 显示帮助（打印此命令列表）\n";
    std::cout << "  exit        - 退出程序（自动卸载已挂载的磁盘）\n";
}

/**
 * @brief 主函数，处理命令行输入和用户交互
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组（第2个参数为磁盘文件路径）
 * @return 程序退出状态码（0为正常退出）
 */
int main(int argc, char* argv[]) {
    // 检查命令行参数是否正确（需指定磁盘文件路径）
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <磁盘文件>\n";
        return 1;
    }

    // 初始化磁盘文件系统对象（传入磁盘文件路径）
    DiskFS disk(argv[1]);
    // 打印帮助信息，提示用户可执行的命令
    print_help();

    std::string command;  // 存储用户输入的命令
    // 命令循环：持续接收并处理用户命令直到输入exit
    while (true) {
        std::cout << "\n> ";  // 命令提示符
        std::cin >> command;  // 读取命令

        // 处理格式化命令
        if (command == "format") {
            if (disk.format()) {
                std::cout << "格式化成功\n";
            } else {
                std::cout << "格式化失败\n";
            }
        } 
        // 处理挂载命令
        else if (command == "mount") {
            if (disk.mount()) {
                std::cout << "挂载成功\n";
            } else {
                std::cout << "挂载失败\n";
            }
        } 
        // 处理卸载命令
        else if (command == "umount") {
            if (disk.unmount()) {
                std::cout << "卸载成功\n";
            } else {
                std::cout << "卸载失败\n";
            }
        } 
        // 处理显示磁盘信息命令
        else if (command == "info") {
            disk.print_info();
        } 
        // 处理创建文件命令
        else if (command == "create") {
            std::string filename;  // 存储文件名
            std::cin >> filename;  // 读取文件名
            // 调用创建文件方法，返回inode编号（-1表示失败）
            int inode = disk.create_file(filename);
            if (inode != -1) {
                std::cout << "创建文件成功，inode: " << inode << "\n";
            } else {
                std::cout << "创建文件失败\n";
            }
        } 
        // 处理打开文件命令
        else if (command == "open") {
            std::string filename;  // 存储文件名
            std::cin >> filename;  // 读取文件名
            // 调用打开文件方法，返回inode编号（-1表示文件不存在）
            int inode = disk.open_file(filename);
            if (inode != -1) {
                std::cout << "文件打开成功，inode: " << inode << "\n";
            } else {
                std::cout << "文件不存在\n";
            }
        } 
        // 处理读取文件命令
        else if (command == "read") {
            int inode, size;  // inode编号和读取大小
            std::cin >> inode >> size;  // 读取参数
            // 分配缓冲区（+1用于存储字符串结束符）
            char* buffer = new char[size + 1];
            // 调用读取文件方法（从偏移量0开始读取）
            int bytes_read = disk.read_file(inode, buffer, size, 0);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';  // 添加字符串结束符
                std::cout << "读取成功，" << bytes_read << "字节:\n" << buffer << "\n";
            } else if (bytes_read == 0) {
                std::cout << "文件为空或已到末尾\n";
            } else {
                std::cout << "读取失败\n";
            }
            delete[] buffer;  // 释放缓冲区
        } 
        // 处理写入文件命令
        else if (command == "write") {
            int inode;  // inode编号
            std::cin >> inode;  // 读取inode
            std::cin.ignore();  // 忽略输入缓冲区中的换行符（避免影响后续getline）
            std::string content;  // 存储写入内容
            std::getline(std::cin, content);  // 读取整行内容（支持空格）
            // 调用写入文件方法（从偏移量0开始写入）
            int bytes_written = disk.write_file(inode, content.c_str(), content.size(), 0);
            if (bytes_written > 0) {
                std::cout << "写入成功，" << bytes_written << "字节\n";
            } else {
                std::cout << "写入失败\n";
            }
        } 
        // 处理删除文件命令
        else if (command == "delete") {
            std::string filename;  // 存储文件名
            std::cin >> filename;  // 读取文件名
            if (disk.delete_file(filename)) {
                std::cout << "删除文件成功\n";
            } else {
                std::cout << "删除文件失败\n";
            }
        } 
        // 处理列出文件命令
        else if (command == "ls") {
            // 获取文件列表（目录项数组）
            std::vector<DirEntry> entries = disk.list_files();
            std::cout << "文件列表:\n";
            // 遍历目录项，跳过当前目录（.）
            for (const auto& entry : entries) {
                if (entry.inode_num != 0) {  // 跳过.目录（inode 0为根目录）
                    std::cout << "  " << entry.name << " (inode: " << entry.inode_num << ")\n";
                }
            }
        } 
        // 处理帮助命令
        else if (command == "help") {
            print_help();
        } 
        // 处理退出命令
        else if (command == "exit") {
            // 若磁盘已挂载，则先卸载
            if (disk.isMounted()) {
                disk.unmount();
            }
            break;  // 退出循环，结束程序
        } 
        // 处理未知命令
        else {
            std::cout << "未知命令，请输入help查看帮助\n";
        }
    }

    return 0;
}