# cmdkit

cmdkit 是一个 [busybox](https://busybox.net/) 的实现, 集成了许多标准 Unix 工具的功能

同时 cmdkit 也是一个轻量级的 Unix 的 C 库, 您可方便的链接此库以实现相关 Unix 工具的功能

## Compile

```bash
make
```

默认编译可以得到一个子文件夹 `cmdkit/`

```bash
├── cmdkit
│   ├── bin         所有可执行文件
│   ├── include     头文件
│   └── lib         静态链接库
├── src
│   └── ls          ls命令的所有相关文件
│       └── main.c  每个目录下的main.c为入口
└── test            测试程序
```

## Usage

下面假定您已位于本项目的根目录下并且已经完成编译

### Unix 工具

如果您想使用 cmdkit 的 ls 命令, 您可以创建如下软链接以覆盖默认 ls

```bash
ln -s ./cmdkit/bin/ls /usr/local/bin/ls
```

使用结束之后建议您取消此链接

```bash
rm /usr/local/bin/ls
```

### Unix C 库

您可以引入 `cmdkit/include` 下的相关头文件并链接 `cmdkit/lib/libcmdkit.a` 以实现函数调用

```bash
gcc -Icmdkit/include -Lcmdkit/lib main.c -lcmdkit -o main
```

或者您习惯 Makefile

```Makefile
CC = gcc
INCLUDE_DIRS += -Icmdkit/include
LD_LIBRARY_PATH += -Lcmdkit/lib
LDFLAGS += cmdkit

main: main.c
    $(CC) $(INCLUDE_DIRS) $(LD_LIBRARY_PATH) $^ $(LDFLAGS) -o $@
```

## 文档

关于如何使用 cmdkit 的 Unix 工具请参考 [工具文档](https://luzhixing12345.github.io/cmdkit)

关于如何使用相关头文件请参考 [头文件文档](https://luzhixing12345.github.io/cmdkit)

## 参考

- [busybox](https://busybox.net/)