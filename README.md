文件划分

超级块 ｜ 块位图 ｜ inode 位图 ｜ inode 区 ｜ 数据区

# 磁盘模拟文件系统

## 项目结构
- `include/`：头文件目录
  - `disk_fs.h`：核心数据结构与接口定义
  - `command_parser.h`：命令解析接口
- `src/`：源文件目录
  - `disk_init.cpp`：磁盘初始化（格式化、挂载、卸载）
  - `bitmap_ops.cpp`：位图操作（块和inode分配管理）
  - `pos_calc.cpp`：磁盘位置计算（块、inode的偏移量）
  - `block_ops.cpp`：块读写操作
  - `file_ops.cpp`：文件操作（创建、读写、删除等）
  - `command_parser.cpp`：命令解析与执行
  - `main.cpp`：主程序入口
- `test/`：测试代码目录
  - 单元测试与集成测试

## 磁盘布局
超级块 ｜ 块位图 ｜ inode 位图 ｜ inode 区 ｜ 数据区


## 编译与使用
- 编译主程序：`make sim_disk`
- 运行测试：`make run_tests && ./run_tests`
- 启动模拟器：`./sim_disk disk.img`

## 支持命令
- `format`：格式化磁盘
- `mount`/`umount`：挂载/卸载磁盘
- `create <文件名>`：创建文件
- `open <文件名>`：获取文件inode
- `read <inode> <大小>`：读取文件内容
- `write <inode> <内容>`：写入文件内容
- `delete <文件名>`：删除文件
- `ls`：列出所有文件
- `info`：显示磁盘信息
- `help`：查看帮助
- `exit`：退出程序


