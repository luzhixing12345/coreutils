# Unixbox

Unixbox 是一个 [busybox](https://busybox.net/) 的实现, 集成了许多标准 Unix 工具的功能

同时 Unixbox 也是一个轻量级的 Unix 的 C 库, 您可方便的链接此库以实现相关 Unix 工具的功能

- Unix 工具的 C 函数级调用
- 命令行参数处理
- 字符串处理: 包括字符串转换,查找,替换,分割,格式化等操作

## Compile

```bash
make
```

默认编译可以得到一个子文件夹 `Unixbox/`

```bash
├── Unixbox
│   ├── bin         所有可执行文件
│   ├── include     头文件
│   └── lib         静态链接库
├── src
│   ├── ls
│   └── ping
└── test            测试程序
```

## Usage

下面假定您已位于本项目的根目录下并且已经完成编译

### Unix 工具

如果您想使用 Unixbox 的 ls 命令, 您可以创建如下软链接以覆盖默认 ls

```bash
ln -s ./Unixbox/bin/ls /usr/local/bin/ls
```

使用结束之后建议您取消此链接

```bash
rm /usr/local/bin/ls
```

### Unix C 库

您可以引入 `Unixbox/include` 下的相关头文件并链接 `Unixbox/lib/libUnixbox.a` 以实现函数调用

```bash
gcc -IUnixbox/include -LUnixbox/lib main.c -lUnixbox -o main
```

或者您习惯 Makefile

```Makefile
CC = gcc
INCLUDE_DIRS += -IUnixbox/include
LD_LIBRARY_PATH += -LUnixbox/lib
LDFLAGS += Unixbox

main: main.c
    $(CC) $(INCLUDE_DIRS) $(LD_LIBRARY_PATH) $^ $(LDFLAGS) -o $@
```

## 文档

关于如何使用 Unixbox 的 Unix 工具请参考 [工具文档](https://luzhixing12345.github.io/Unixbox)

关于如何使用相关头文件请参考 [头文件文档](https://luzhixing12345.github.io/Unixbox)

## 参考

- [busybox](https://busybox.net/)
- [Boost](https://www.boost.org/)
- [filesystem](https://en.cppreference.com/w/cpp/filesystem)
- [Poco](https://github.com/pocoproject/poco)
- [Asio](https://think-async.com/Asio/)
- [Cairo](https://www.cairographics.org/)
- [QT Graphics View Framework](https://doc.qt.io/qt-6/graphicsview.html)
- [AGG](https://github.com/ghaerr/agg-2.6)
- [skia](https://skia.org/)
- [ImageMagick](https://imagemagick.org/script/magick++.php)
- [libuv](https://github.com/libuv/libuv)
- [Simple-web-server](https://github.com/eidheim/Simple-Web-Server)
- [Crow](https://crowcpp.org/master/)