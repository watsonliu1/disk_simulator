#include "include/disk_fs.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>  // 用于测试断言

// 原有 print_help() 函数保持不变
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

// 测试用例实现
bool run_tests(DiskFS& disk) {
    int test_count = 0;
    int pass_count = 0;
    std::cout << "\n===== 开始自动测试 =====" << std::endl;

    // 测试1: 格式化磁盘
    test_count++;
    bool format_ok = disk.format();
    std::cout << "测试" << test_count << "(格式化): " << (format_ok ? "通过" : "失败") << std::endl;
    if (format_ok) pass_count++;

    // 测试2: 挂载磁盘
    test_count++;
    bool mount_ok = disk.mount();
    std::cout << "测试" << test_count << "(挂载): " << (mount_ok ? "通过" : "失败") << std::endl;
    if (mount_ok) pass_count++;

    // 测试3: 创建文件
    test_count++;
    int temp_inode = disk.create_file("test1.txt");  // 先用int接收检查是否失败
    uint32_t inode1 = static_cast<uint32_t>(temp_inode);  // 转换为无符号类型
    bool create_ok = (temp_inode != -1);  // 基于int类型判断是否创建成功
    std::cout << "测试" << test_count << "(创建文件): " << (create_ok ? "通过" : "失败") << std::endl;
    if (create_ok) pass_count++;

    // 测试4: 禁止创建同名文件
    test_count++;
    int inode_dup = disk.create_file("test1.txt");
    bool no_dup_ok = (inode_dup == -1);
    std::cout << "测试" << test_count << "(禁止同名文件): " << (no_dup_ok ? "通过" : "失败") << std::endl;
    if (no_dup_ok) pass_count++;

    // 测试5: 写入文件
    test_count++;
    std::string content = "hello, disk fs!";
    int write_size = disk.write_file(inode1, content.c_str(), content.size(), 0);
    bool write_ok = (write_size == (int)content.size());
    std::cout << "测试" << test_count << "(写入文件): " << (write_ok ? "通过" : "失败") << std::endl;
    if (write_ok) pass_count++;

    // 测试6: 读取文件
    test_count++;
    char* read_buf = new char[content.size() + 1];
    int read_size = disk.read_file(inode1, read_buf, content.size(), 0);
    read_buf[read_size] = '\0';
    bool read_ok = (read_size == (int)content.size() && std::string(read_buf) == content);
    std::cout << "测试" << test_count << "(读取文件): " << (read_ok ? "通过" : "失败") << std::endl;
    if (read_ok) pass_count++;
    delete[] read_buf;

    // 测试7: 列出文件
    test_count++;
    std::vector<DirEntry> entries = disk.list_files();
    bool ls_ok = false;
    for (const auto& entry : entries) {
        if (entry.valid && std::string(entry.name) == "test1.txt" && entry.inode_num == inode1) {
            ls_ok = true;
            break;
        }
    }
    std::cout << "测试" << test_count << "(列出文件): " << (ls_ok ? "通过" : "失败") << std::endl;
    if (ls_ok) pass_count++;

    // 测试8: 删除文件
    test_count++;
    bool delete_ok = disk.delete_file("test1.txt");
    std::cout << "测试" << test_count << "(删除文件): " << (delete_ok ? "通过" : "失败") << std::endl;
    if (delete_ok) pass_count++;

    // 测试9: 验证文件已删除
    test_count++;
    int deleted_inode = disk.open_file("test1.txt");
    bool verify_delete_ok = (deleted_inode == -1);
    std::cout << "测试" << test_count << "(验证删除): " << (verify_delete_ok ? "通过" : "失败") << std::endl;
    if (verify_delete_ok) pass_count++;

    // 测试10: 卸载磁盘
    test_count++;
    bool unmount_ok = disk.unmount();
    std::cout << "测试" << test_count << "(卸载): " << (unmount_ok ? "通过" : "失败") << std::endl;
    if (unmount_ok) pass_count++;

    // 输出测试总结
    std::cout << "\n===== 测试总结 =====" << std::endl;
    std::cout << "总测试数: " << test_count << std::endl;
    std::cout << "通过数: " << pass_count << std::endl;
    std::cout << "失败数: " << (test_count - pass_count) << std::endl;

    return (test_count == pass_count);
}

int main(int argc, char* argv[]) {
    // 支持测试模式: ./sim_disk <磁盘文件> --test
    if (argc == 3 && std::string(argv[2]) == "--test") {
        DiskFS disk(argv[1]);
        bool all_passed = run_tests(disk);
        return all_passed ? 0 : 1;
    }

    // 原有交互模式
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <磁盘文件>\n";
        std::cerr << "测试模式: " << argv[0] << " <磁盘文件> --test\n";
        return 1;
    }

    DiskFS disk(argv[1]);
    print_help();

    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::cin >> command;

        // 原有命令处理逻辑保持不变...
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
            if (disk.isMounted()) {
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