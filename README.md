# xbox

xbox 是一个 GNU coreutils 的实现

xbox 是一个轻量级的 Unix 命令行程序的函数库, 可以实现 C 函数级调用

xbox 是一个轻量级的基础 C 库, 包括命令行参数处理和字符串处理

## 编译和使用

编译

```bash
make
```

安装: 意味着将以高优先级使用 xbox 编译得到的程序

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

[busybox源码构建]()

## 参考

- [busybox](https://busybox.net/)
- [glibc](https://github.com/bminor/glibc)
- [gnu-coreutils](https://www.maizure.org/projects/decoded-gnu-coreutils/)
- [Boost](https://www.boost.org/)
- [filesystem](https://en.cppreference.com/w/cpp/filesystem)
- [Poco](https://github.com/pocoproject/poco)
- [Asio](https://think-async.com/Asio/)
- [libuv](https://github.com/libuv/libuv)
- [Simple-web-server](https://github.com/eidheim/Simple-Web-Server)
- [Crow](https://crowcpp.org/master/)