#include "../include/disk_fs.h"
#include <iostream>
#include <string>
#include <vector>

bool run_tests(DiskFS& disk) 
{
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
    int temp_inode = disk.create_file("test1.txt");
    uint32_t inode1 = static_cast<uint32_t>(temp_inode);
    bool create_ok = (temp_inode != -1);
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

    std::cout << "\n===== 测试总结 =====" << std::endl;
    std::cout << "总测试数: " << test_count << std::endl;
    std::cout << "通过数: " << pass_count << std::endl;
    std::cout << "失败数: " << (test_count - pass_count) << std::endl;

    return (test_count == pass_count);
}

int main() {
    DiskFS disk("test_disk.img");

    run_tests(disk);
    return 0;
}