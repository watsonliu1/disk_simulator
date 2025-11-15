# CXX = g++
# CXXFLAGS = -std=c++11 -Iinclude -Wall -Wextra -pthread
# LDFLAGS = -pthread

# TARGET = sim_disk
# SRCS = src/main.cpp src/disk_init.cpp src/bitmap_ops.cpp src/pos_calc.cpp \
#        src/block_ops.cpp src/file_ops.cpp src/command_parser.cpp
# OBJS = $(SRCS:.cpp=.o)

# TEST_TARGET = test_disk
# TEST_SRCS = test/test_main.cpp
# TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# all: $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# test: $(TEST_OBJS) $(filter-out src/main.o, $(OBJS))
# 	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS) $(filter-out src/main.o, $(OBJS)) $(LDFLAGS)

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET) test_disk.img disk.img

# .PHONY: all test clean

CXX = g++
CXXFLAGS = -std=c++11 -Iinclude -Wall -Wextra -pthread
LDFLAGS = -pthread

# 核心目标定义
TARGET = sim_disk               # 主程序
SO_LIB = libdiskfs.so           # SO库
TEST_TARGET = test_disk         # 测试程序（仅make test时生成）

# 源文件分类
# 1. 主程序及底层功能源文件（不含测试代码）
SRCS = src/main.cpp src/disk_init.cpp src/bitmap_ops.cpp src/pos_calc.cpp \
       src/block_ops.cpp src/file_ops.cpp src/command_parser.cpp
# 2. 仅底层功能源文件（用于生成SO库，排除主程序入口）
SO_SRCS = $(filter-out src/main.cpp, $(SRCS))
# 3. 测试程序源文件（仅make test时编译）
TEST_SRCS = test/test_main.cpp

# 目标文件分类
OBJS = $(SRCS:.cpp=.o)                  # 主程序及底层功能目标文件
SO_OBJS = $(SO_SRCS:.cpp=.o)            # SO库依赖的目标文件
TEST_OBJS = $(TEST_SRCS:.cpp=.o)        # 测试程序目标文件（仅make test时生成）

# 默认目标：仅生成SO库和主程序（不生成测试文件）
all: $(SO_LIB) $(TARGET)

# 生成SO库（主程序和测试程序都依赖它）
$(SO_LIB): $(SO_OBJS)
	$(CXX) -shared -fPIC -o $@ $(SO_OBJS) $(LDFLAGS)
	@echo "SO库生成完成: $(SO_LIB)"

# 生成主程序（依赖SO库）
$(TARGET): src/main.o $(SO_LIB)
	$(CXX) $(CXXFLAGS) -o $@ src/main.o -L. -ldiskfs $(LDFLAGS)
	@echo "主程序生成完成: $(TARGET)"

# 测试目标：仅执行make test时生成测试程序（依赖SO库和测试目标文件）
test: $(SO_LIB) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS) -L. -ldiskfs $(LDFLAGS)
	@echo "测试程序生成完成: $(TEST_TARGET)"

# 编译规则：
# - 底层功能文件（SO_OBJS）加-fPIC（用于SO库）
# - 主程序和测试文件按常规编译
%.o: %.cpp
	$(if $(filter $(SO_OBJS), $@), \
		$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@, \
		$(CXX) $(CXXFLAGS) -c $< -o $@ \
	)

# 清理目标：删除所有生成文件（含测试文件）
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET) $(SO_LIB) test_disk.img disk.img
	@echo "清理完成"

.PHONY: all test clean