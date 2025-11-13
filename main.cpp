#include "include/disk_fs.h"
#include <iostream>
#include <string>
#include <vector>

void print_help() {
    std::cout << "磁盘模拟文件系统命令:\n";
    std::cout << "  format      - 格式化磁盘\n";
    std::cout << "  mount       - 挂载磁盘\n";
    std::cout << "  umount      - 卸载磁盘\n";
    std::cout << "  info        - 显示磁盘信息\n";
    std::cout << "  create <文件名> - 创建文件\n";
    std::cout << "  open <文件名>   - 打开文件(获取inode)\n";
    std::cout << "  read <inode> <大小> - 读取文件\n";
    std::cout << "  write <inode> <内容> - 写入文件\n";
    std::cout << "  delete <文件名> - 删除文件\n";
    std::cout << "  ls          - 列出文件\n";
    std::cout << "  help        - 显示帮助\n";
    std::cout << "  exit        - 退出\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <磁盘文件>\n";
        return 1;
    }

    DiskFS disk(argv[1]);
    print_help();

    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::cin >> command;

        if (command == "format") {
            if (disk.format()) {
                std::cout << "格式化成功\n";
            } else {
                std::cout << "格式化失败\n";
            }
        } 
        else if (command == "mount") {
            if (disk.mount()) {
                std::cout << "挂载成功\n";
            } else {
                std::cout << "挂载失败\n";
            }
        } 
        else if (command == "umount") {
            if (disk.unmount()) {
                std::cout << "卸载成功\n";
            } else {
                std::cout << "卸载失败\n";
            }
        } 
        else if (command == "info") {
            disk.print_info();
        } 
        else if (command == "create") {
            std::string filename;
            std::cin >> filename;
            int inode = disk.create_file(filename);
            if (inode != -1) {
                std::cout << "创建文件成功，inode: " << inode << "\n";
            } else {
                std::cout << "创建文件失败\n";
            }
        } 
        else if (command == "open") {
            std::string filename;
            std::cin >> filename;
            int inode = disk.open_file(filename);
            if (inode != -1) {
                std::cout << "文件打开成功，inode: " << inode << "\n";
            } else {
                std::cout << "文件不存在\n";
            }
        } 
        else if (command == "read") {
            int inode, size;
            std::cin >> inode >> size;
            char* buffer = new char[size + 1];
            int bytes_read = disk.read_file(inode, buffer, size, 0);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "读取成功，" << bytes_read << "字节:\n" << buffer << "\n";
            } else if (bytes_read == 0) {
                std::cout << "文件为空或已到末尾\n";
            } else {
                std::cout << "读取失败\n";
            }
            delete[] buffer;
        } 
        else if (command == "write") {
            int inode;
            std::cin >> inode;
            std::cin.ignore();  // 忽略换行符
            std::string content;
            std::getline(std::cin, content);
            int bytes_written = disk.write_file(inode, content.c_str(), content.size(), 0);
            if (bytes_written > 0) {
                std::cout << "写入成功，" << bytes_written << "字节\n";
            } else {
                std::cout << "写入失败\n";
            }
        } 
        else if (command == "delete") {
            std::string filename;
            std::cin >> filename;
            if (disk.delete_file(filename)) {
                std::cout << "删除文件成功\n";
            } else {
                std::cout << "删除文件失败\n";
            }
        } 
        else if (command == "ls") {
            std::vector<DirEntry> entries = disk.list_files();
            std::cout << "文件列表:\n";
            for (const auto& entry : entries) {
                if (entry.inode_num != 0) {  // 跳过.目录
                    std::cout << "  " << entry.name << " (inode: " << entry.inode_num << ")\n";
                }
            }
        } 
        else if (command == "help") {
            print_help();
        } 
        else if (command == "exit") {
            if (disk.isMounted()) {  // 修正函数调用
                disk.unmount();
            }
            break;
        } 
        else {
            std::cout << "未知命令，请输入help查看帮助\n";
        }
    }

    return 0;
}