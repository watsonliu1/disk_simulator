#include "../include/command_parser.h"
#include <iostream>
#include <sstream>
#include <vector>

void CommandParser::print_help() const {
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

bool CommandParser::execute_command(const std::string& command_line) {
    std::istringstream iss(command_line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    if (tokens.empty()) return true;

    if (tokens[0] == "format") {
        if (disk.format()) {
            std::cout << "格式化成功\n";
        } else {
            std::cout << "格式化失败\n";
        }
    } else if (tokens[0] == "mount") {
        if (disk.mount()) {
            std::cout << "挂载成功\n";
        } else {
            std::cout << "挂载失败\n";
        }
    } else if (tokens[0] == "umount") {
        if (disk.unmount()) {
            std::cout << "卸载成功\n";
        } else {
            std::cout << "卸载失败\n";
        }
    } else if (tokens[0] == "info") {
        disk.print_info();
    } else if (tokens[0] == "create") {
        if (tokens.size() < 2) {
            std::cout << "用法: create <文件名>\n";
            return false;
        }
        int inode = disk.create_file(tokens[1]);
        if (inode != -1) {
            std::cout << "创建文件成功，inode: " << inode << "\n";
        } else {
            std::cout << "创建文件失败\n";
        }
    } else if (tokens[0] == "open") {
        if (tokens.size() < 2) {
            std::cout << "用法: open <文件名>\n";
            return false;
        }
        int inode = disk.open_file(tokens[1]);
        if (inode != -1) {
            std::cout << "文件打开成功，inode: " << inode << "\n";
        } else {
            std::cout << "文件不存在\n";
        }
    } else if (tokens[0] == "read") {
        if (tokens.size() < 3) {
            std::cout << "用法: read <inode> <大小>\n";
            return false;
        }
        int inode = std::stoi(tokens[1]);
        int size = std::stoi(tokens[2]);
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
    } else if (tokens[0] == "write") {
        if (tokens.size() < 2) {
            std::cout << "用法: write <inode> <内容>\n";
            return false;
        }
        int inode = std::stoi(tokens[1]);
        std::string content;
        size_t pos = command_line.find(tokens[0]) + tokens[0].size();
        pos = command_line.find(tokens[1], pos) + tokens[1].size();
        if (pos < command_line.size()) {
            content = command_line.substr(pos + 1);
        }
        int bytes_written = disk.write_file(inode, content.c_str(), content.size(), 0);
        if (bytes_written > 0) {
            std::cout << "写入成功，" << bytes_written << "字节\n";
        } else {
            std::cout << "写入失败\n";
        }
    } else if (tokens[0] == "delete") {
        if (tokens.size() < 2) {
            std::cout << "用法: delete <文件名>\n";
            return false;
        }
        if (disk.delete_file(tokens[1])) {
            std::cout << "删除文件成功\n";
        } else {
            std::cout << "删除文件失败\n";
        }
    } else if (tokens[0] == "ls") {
        std::vector<DirEntry> entries = disk.list_files();
        std::cout << "文件列表:\n";
        for (const auto& entry : entries) {
            if (entry.inode_num != 0) {
                std::cout << "  " << entry.name << " (inode: " << entry.inode_num << ")\n";
            }
        }
    } else if (tokens[0] == "help") {
        print_help();
    } else if (tokens[0] == "exit") {
        if (disk.isMounted()) {
            disk.unmount();
        }
        exit(0);
    } else {
        std::cout << "未知命令，请输入help查看帮助\n";
    }
    return true;
}