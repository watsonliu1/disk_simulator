#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>
#include "disk_fs.h"

class CommandParser {
private:
    DiskFS& disk;

public:
    CommandParser(DiskFS& disk_fs) : disk(disk_fs) {}

    bool execute_command(const std::string& command_line);
    void print_help() const;
};

#endif // COMMAND_PARSER_H