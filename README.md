# xbox

xbox 是一个 [busybox](https://busybox.net/) 的实现, 集成了许多标准 Unix 工具的功能

xbox 是一个轻量级的 Unix 命令行程序的函数库, 可以实现命令的 C 函数级调用

xbox 是一个轻量级的基础 C 库, 包括命令行参数处理和字符串处理

## 编译和使用

编译

```bash
make
```

安装: 意味着将以高优先级使用 xbox 编译得到的程序而非默

```bash
sudo make install
```

卸载

```bash
sudo make uninstall
```

### Linux C 库

默认编译可以得到一个子文件夹 `xbox/`

```bash
├── xbox
│   ├── bin         所有可执行文件
│   ├── include     头文件
│   └── lib         静态链接库
├── src             源代码实现
└── test            测试程序
```

您可以引入 `xbox/include` 下的相关头文件并链接 `xbox/lib/libxbox.a` 以实现函数调用

```bash
gcc -Ixbox/include -Lxbox/lib main.c -lxbox -o main
```

或者您习惯 Makefile

```Makefile
CC = gcc
INCLUDE_DIRS += -Ixbox/include
LD_LIBRARY_PATH += -Lxbox/lib
LDFLAGS += xbox

main: main.c
    $(CC) $(INCLUDE_DIRS) $(LD_LIBRARY_PATH) $^ $(LDFLAGS) -o $@
```

## 文档

关于如何使用 xbox 的 Linux 工具请参考 [工具文档](https://luzhixing12345.github.io/xbox)

关于如何使用相关头文件请参考 [头文件文档](https://luzhixing12345.github.io/xbox)

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