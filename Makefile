

CC = gcc
CFLAGS = -Wall -O2
TARGET = Unixbox
SRC_PATH = src
# 搜索的后缀(.cpp -> .h)
SRC_EXT = c
# 测试文件夹
TEST_PATH = test
# 项目名字(库)
RELEASE = $(TARGET)
# 编译得到的静态库的名字(如果需要)
LIB = lib$(TARGET).a
# ------------------------- #

# 递归的搜索所有SRC_PATH目录下的.SRC_TXT类型的文件
# rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard,$d/,$2) \
# 						$(filter $2, $d))

# SRC = $(call rwildcard, $(SRC_PATH), %.$(SRC_EXT))
# OBJ = $(SRC:$(SRC_EXT)=o)
# HEADER = $(SRC:$(SRC_EXT)=h)
# EXE = $(OBJ:%.o=%)
UTILS_HEADER = $(wildcard $(SRC_PATH)/*.h)

DIRECTORY = $(filter-out $(UTILS_HEADER),$(wildcard $(SRC_PATH)/*))

all: $(DIRECTORY)
	@for dir in $^ ; do \
		$(MAKE) -C $$dir ; \
	done
	
%.o: %.c %.h $(UTILS_HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	$(CC) $(CFLAGS) -o $@ $<

# ------------------------- #
#          使用方法
# ------------------------- #
.PHONY: clean clean_all lib release tar all

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


clean: $(DIRECTORY)
	@for dir in $^ ; do \
		$(MAKE) -C $$dir clean; \
	done

clean_all:
	rm -r $(TARGET)
	$(MAKE) clean

lib: $(obj)
	@ar rsv lib$(TARGET).a $(obj)

release:
	$(MAKE) lib
	mkdir -p $(RELEASE)/bin
	mkdir -p $(RELEASE)/include/$(RELEASE)
	mkdir -p $(RELEASE)/lib
	@cp -v $(EXE) $(RELEASE)/bin | awk -v format=$(CP_FORMAT) '{printf format, $$1, $$3}'
	@cp -v $(HEADER) $(RELEASE)/include/$(RELEASE) | awk -v format=$(CP_FORMAT) '{printf format, $$1, $$3}'
	@mv -v $(LIB) $(RELEASE)/lib | awk -v format=$(MV_FORMAT) '{printf format, $$2, $$4}'

tar:
	tar -cvf $(TARGET).tar $(RELEASE)/

