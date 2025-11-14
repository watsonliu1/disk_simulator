#include "../include/disk_fs.h"
#include "../include/command_parser.h"
#include <iostream>
#include <string>
#include <cassert>

int main(int argc, char* argv[]) 
{
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <磁盘文件>\n";
        std::cerr << "测试模式: " << argv[0] << " <磁盘文件> --test\n";
        return 1;
    }

    DiskFS disk(argv[1]);
    CommandParser parser(disk);
    parser.print_help();

    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, command);
        if (command == "exit") break;
        parser.execute_command(command);
    }

    if (disk.isMounted()) {
        disk.unmount();
    }
    return 0;
}