

CC = gcc
CFLAGS = -Wall -Wunused -Werror -Wformat-security -Wshadow -Wpedantic -Wstrict-aliasing -Wuninitialized -Wnull-dereference -Wformat=2
TARGET = coreutils
SRC_PATH = src
# 搜索的后缀(.cpp -> .h)
SRC_EXT = c
# 头文件
INCLUDE_PATH = include
LDFLAGS = 
# 测试文件夹
TEST_PATH = test
# 项目名字(库)
RELEASE = $(TARGET)
# 编译得到的静态库的名字(如果需要)
LIB = lib$(TARGET).a
# ------------------------- #

rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard,$d/,$2) \
                        $(filter $2, $d))

SRC := $(wildcard $(SRC_PATH)/*.$(SRC_EXT))
OBJ = $(SRC:$(SRC_EXT)=o)
THIRD_LIB = $(call rwildcard, $(INCLUDE_PATH), %.$(SRC_EXT))
THIRD_LIB_OBJ = $(THIRD_LIB:$(SRC_EXT)=o)
EXE = $(OBJ:%.o=%)

CFLAGS += -I$(INCLUDE_PATH)

ifeq ($(MAKECMDGOALS),debug)
CFLAGS+=-g
endif

all: $(EXE)

debug: all

$(EXE): %: %.o $(THIRD_LIB_OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# dircolors 数据库中字符串过长
$(SRC_PATH)/dircolors.o: $(SRC_PATH)/dircolors.c
	$(CC) $(CFLAGS) -Wno-overlength-strings -c $< -o $@

# ------------------------- #
#          使用方法
# ------------------------- #
.PHONY: clean clean_all lib release tar all test

# make : 编译
# make clean: 清除编译的中间文件
# make clean_all: 清除所有编译结果
# make lib: 将所有obj整合到一个.a
# make release: 导出release库
# make tar: 打包release
# make install: 安装release库
# make uninstall: 卸载release库

# Define variables for formatting
CP_FORMAT = "[cp]\t%-20s -> %s\n"
MV_FORMAT = "[mv]\t%-20s -> %s\n"

test:
	$(MAKE) clean
	$(MAKE) debug -j4
	python test/test.py

clean:
	rm -f $(EXE) $(OBJ) $(THIRD_LIB_OBJ)
release:
	$(MAKE) -j4
	mkdir $(RELEASE)
	@cp $(EXE) $(RELEASE)/ 
	tar -cvf $(TARGET).tar $(RELEASE)/