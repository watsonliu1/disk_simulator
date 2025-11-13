CXX = g++
CXXFLAGS = -std=c++11 -Iinclude -Wall -Wextra
LDFLAGS = 

# 目标文件
TARGET = sim_disk

# 源文件
SRCS = main.cpp src/disk_fs.cpp
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# 编译
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean