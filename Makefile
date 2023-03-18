

CC = gcc
CFLAGS = -Wall -O2
TARGET = cmdkit
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

# 安装的路径
prefix = /usr/local
libdir = $(prefix)/lib
includedir = $(prefix)/include

# 递归的搜索所有SRC_PATH目录下的.SRC_TXT类型的文件
rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard,$d/,$2) \
						$(filter $2, $d))

src = $(call rwildcard, $(SRC_PATH), %.$(SRC_EXT))
obj = $(src:$(SRC_EXT)=o)
header = $(src:$(SRC_EXT)=h)

all: $(obj)
	$(CC) $(CFLAGS) $(LD_LIBRARY_PATH) $^ $(LDFLAGS) -o $@

$(obj): %.o : %.cpp


# ------------------------- #
#          使用方法
# ------------------------- #
.PHONY: clean clean_all lib release tar install uninstall all

# make : 编译
# make clean: 清除编译的中间文件
# make clean_all: 清除所有编译结果
# make lib: 将所有obj整合到一个.a
# make release: 导出release库
# make tar: 打包release
# make install: 安装release库
# make uninstall: 卸载release库

# ------------------------- #
SILENCE = 2>/dev/null # 安静的删除文件,没有多余的提示信息

clean:
	@-rm $(obj) $(TARGET) $(SILENCE)
	@echo "[clean]"

clean_all:
	@-rm -r $(RELEASE) $(SILENCE)
	@-rm $(obj) $(LIB) $(TARGET) $(SILENCE)
	@echo "clean all compiled files"

lib: $(obj)
	ar rsv lib$(TARGET).a $(obj)

release:
	$(MAKE) lib
	mkdir -p $(RELEASE)/include/$(RELEASE)
	mkdir -p $(RELEASE)/lib
	cp $(header) $(RELEASE)/include/$(RELEASE)
	mv $(LIB) $(RELEASE)/lib
	@echo "\n[finished]: release package in $(RELEASE)/\n"

tar:
	tar -cvf $(TARGET).tar $(RELEASE)/

install:
	mkdir -p $(libdir) $(includedir)/$(RELEASE)
	cp $(LIB) $(libdir)/
	cp $(header) $(includedir)/$(RELEASE)

uninstall:
	rm $(libdir)/$(LIB)
	rm -r $(includedir)/$(RELEASE)
