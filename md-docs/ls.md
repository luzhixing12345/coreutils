
# ls

应该算是最常用的命令之一了, 用于列出目录下文件信息

C 标准库中提供了对于目录的操作函数, 下面是一个极简的 ls 实现

```c
#include <stdio.h>

#include <dirent.h>
#include <sys/types.h>

int main(int argc, char** argv) {
    DIR* dir;
    struct dirent* entry;

    char* path = ".";  // 目录路径
    dir = opendir(path);

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
    return 0;
}
```

opendir 打开文件夹, 使用 readdir 依次读取每一个目录文件项并输出. 但显然 ls 的细节要多一些, 包括可以很明显的看到输出的格式是对其的, 是有排序的, 是有根据文件类型做颜色上的区分的, 那么下面具体介绍一下相关的实现细节

## 终端宽度与输出格式

相信用户很容易就可以注意到在不同的终端宽度下 ls 的显示效果有所不同

![20230918104324](https://raw.githubusercontent.com/learner-lu/picbed/master/20230918104324.png)

首先文件名的默认排序方式按照字典序, 当有多排时**优先竖排, 其次横排**. 对于输出格式的计算方式为:

1. 获取当前终端的宽度 W 和 当前目录下所有文件名的长度 length[M] 用于后续计算
2. 预计使用 1 行显示, 计算组合长度是否小于 W
3. 如果不能则考虑 N 行显示, **计算 N 个对齐元素的最大长度作为当前列的宽度**, 然后组合所有列计算总宽度与 W 比较
4. N => N+1 直至满足条件
5. 边界情况是当 N 等同当前目录文件数量 M, 即所有文件一列排序, 此时不再需要考虑对齐以及能否排下, 直接竖排输出即可

```bash
+---------------------------------------------------------------------------------+
|                                                                                 |
|   +-----------+   +-----------+  +-----------+   +-----------+   +-----------+  |
|   |    1      |   |    3      |  |     5     |   |     7     |   |     9     |  |
|   |           |   |           |  |           |   |           |   |           |  |
|   +-----------+   +-----------+  +-----------+   +-----------+   +-----------+  |
|                                                                                 |
|   +-----------+   +-----------+  +-----------+   +-----------+   +-----------+  |
|   |    2      |   |    4      |  |     6     |   |     8     |   |     10    |  |
|   |           |   |           |  |           |   |           |   |           |  |
|   +-----------+   +-----------+  +-----------+   +-----------+   +-----------+  |
|                                                                                 |
+---------------------------------------------------------------------------------+
+-----------------------------------------------------------------+
|                                                                 |
|   +--+--------+   +-+---------+   +-+---------+   +-----------+ |
|   |  |  1     |   | | 4       |   | | 7       |   | 10        | |
|   |  |        |   | |         |   | |         |   |           | |
|   +-----------+   +-----------+   +-----------+   +-----------+ |
|      |              |               |                           |
|   +-----------+   +-----------+   +-----------+                 |
|   |  |  2     |   | | 5       |   | | 8       |                 |
|   |  |        |   | |         |   | |         |                 |
|   +-----------+   +-----------+   +-----------+                 |
|      |              |               |                           |
|   +-----------+   +-----------+   +-----------+                 |
|   |  |  3     |   | | 6       |   | | 9       |                 |
|   |  v        |   | v         |   | v         |                 |
|   +--+--------+   +-+---------+   +-+---------+                 |
|                                                                 |
+-----------------------------------------------------------------+
```

### 终端宽度

获取终端宽度的方式也很简单, 使用 ioctl 即可

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, const char **argv) {
    struct winsize ws;

    // 获取当前终端窗口的大小
    // TIOCGWINSZ -> get
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }

    printf("当前终端窗口大小:行数: %d ,列数: %d\n", ws.ws_row, ws.ws_col);

    // ws.ws_col = ws.ws_col / 2;

    // TIOCSWINSZ -> set
    // if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws) == -1) {
    //     perror("ioctl");
    //     return 1;
    // }

    return 0;
}
```

> 注释部分是设置终端宽度

但值的一提的是此时查询的是标准输出所在的终端信息, 如果标准输出被修改, 比如使用重定位或管道

```bash
ls > a.txt
ls | grep a
```

此时如果依然使用 ioctl 获取 STDOUT 的信息则会报错 `ioctl: Inappropriate ioctl for device`, **因此 ls 在此时的做法是竖排输出**

![20230918163924](https://raw.githubusercontent.com/learner-lu/picbed/master/20230918163924.png)

> 因此在程序中需要使用 istty(1) 来判断标准输出是否是终端

### tmux 中的宽度

首先需要声明一点的是, tmux 同样只是一个运行在应用层的软件, **因此编写 ls 的时候完全不需要主动考虑 tmux 多开窗口情况, 这是 tmux 本身需要处理的**

多开窗口时, 此时 ioctl 得到的结果分别是 60 和 59, 刚好是 120 - 1(竖线像素) 的平分

![20230918164411](https://raw.githubusercontent.com/learner-lu/picbed/master/20230918164411.png)

> 下面的内容涉及到关于 pty 相关的一些知识, 读者可以参考 [【技术杂谈】shell和terminal](https://www.bilibili.com/video/BV16A411675V/)

传统的 terminal 每个终端只有一组 pty master 和 pty slave, pty master 再交由 X 渲染页面, pty slave 则与 shell 连接, 整体流程如下所示

![20230918202747](https://raw.githubusercontent.com/learner-lu/picbed/master/20230918202747.png)

tmux 的基本思路是在背后维护多个 pty master 和 pty slave, 每一组 pty 都有自己的会话, 统一发送给 tmux 然后 tmux 负责将不同的会话渲染到同一个 tty 页面当中, **它可以通过 pty master 控制终端的大小, 以便它们可以并排显示(一个窗口中的多个窗格)**, 结构如下所示

![20230918202912](https://raw.githubusercontent.com/learner-lu/picbed/master/20230918202912.png)

tmux 实际上是一个客户端服务器应用程序,因为用户可能希望同时从多个客户端访问相同的 tmux 会话.所以它的结构实际上更像是这样的, tmux 服务器维护一组与其窗格相对应的 pty master.客户端通过 unix 域套接字(通常位于 `/tmp/tmux-$UID/default` 比如 `/tmp/tmux-1000/default`)连接到 tmux 服务器以发送输入和接收输出, 如下所示

![20230918202927](https://raw.githubusercontent.com/learner-lu/picbed/master/20230918202927.png)

> 关于这部分的代码可以参见: [tmux tty.c](https://github.com/tmux/tmux/blob/master/tty.c)

---

所以简而言之, tmux 背后对应多个 pty, tmux 会根据窗口的大小对其进行 resize, 因此执行 ls 是所探查的 ioctl 返回的终端宽度并不是看到的 window 的宽度, 而是 pane 所对应的 pty 的宽度, 最后执行的输出结果在交由 tmux 显示在对应的 pane 的位置

![20230918164411](https://raw.githubusercontent.com/learner-lu/picbed/master/20230918164411.png)

## 颜色

解决宽度和显示的问题之后, 很明显要解决的是彩色输出的问题, 那么这里自然而言的引出两个问题:

1. 如何在终端中输出带颜色的文字?
2. ls 对于不同文件类型的颜色是固定的么? 由什么控制? 如何修改?

### 虚拟终端序列

微软的文档 [virtual-terminal-sequences](https://learn.microsoft.com/zh-cn/windows/console/console-virtual-terminal-sequences) 笔者认为已经写的很详细了, 下面来做一个关于彩色文字显示的简要概括

如果希望输出彩色文字, 可以在其前后添加形如 `ESC [ <n> m` 的控制序列字符串, 其中 ESC 指 \033 `<n>` 以表示不同的格式设置模式, 比如在控制台运行如下指令可以得到一个红色的字符串 123

```bash
printf "\033[91m123\033[0m\n"
```

根据不同的 n 值可以控制前景色背景色, 现代的绝大部分虚拟终端仿真器都已经支持了比 Windows 控制台提供的 16 种颜色更多的颜色调色板, 以及支持了使用 RGB 的扩展颜色

除了需要在输出时使用对应的控制序列进行彩色文字输出, **还需要注意两个问题**

1. 标准输出是否是终端

   如果输出并不是终端则不使用控制序列, 比如 `ls > a.txt`, 此时如果也将虚拟控制序列输出到文件中则会出现乱码, 因为该序列是由 terminal 来负责解析和显示的, 不应该直接输出给文本文件

2. 格式化输出长度

   由于 ls 多行需要根据当前列的最大字符长度进行左对齐, 所以考虑如下代码, printf 格式化中都使用了 "%-10s" 来控制至少 10 字符的左对齐, 但由于虚拟控制字符本身占据长度, 因此第二个 printf 的输出对齐并没有按照实际显示的文字 123 进行对齐, 需要进行补齐考虑

   ```c
   #include <stdio.h>

   int main(int argc, const char **argv) {
       printf("%-10sxxx\n", "123");
       printf("%-10sxxx\n", "\033[1;91m123\033[1;0m");
       return 0;
   }
   ```

### LS_COLORS

作为一个高度模块化设计的系统, ls 对应的各个文件类型的颜色肯定不是写死的, 实际上我们可以看到不同的 shell, terminal 都会有一些预制的颜色主题

shell 中有一个名为 `LS_COLORS` 的变量, 改变了控制着如何显示 ls 的颜色, 直接输出的信息比较杂乱, 建议使用 tr 按分隔符 `:` 做换行处理操作

```bash
echo $LS_COLORS | tr ':' '\n'
```

输出结果形如 `key=value` 的键值对, 其中 value 部分就是前文提到的虚拟终端序列, 比如目录 di 为 01 (粗体) 35 (紫色)

key 可分为两类, 一种是缩写形式, 其缩写含义如下所示, 需要探查文件信息来判断相应的文件类型:

- `rs`: 重置(Reset)
- `di`: 目录(Directory)
- `ln`: 符号链接(Symbolic Link)
- `mh`: 多硬连接的文件(Multihardlink)
- `pi`: 命名管道(Named Pipe)
- `so`: 套接字(Socket)
- `do`: 目录(Door): 通常是Solaris操作系统中的一种特殊文件类型,不是标准的UNIX文件类型
- `bd`: 块设备文件(Block Device)
- `cd`: 字符设备文件(Character Device)
- `or`: 孤立的符号链接(orphan symbolic), 符号链接指向不存在的目标文件或目录的符号链接
- `mi`: 缺失的文件(missing file)
- `su`: 设置用户ID位(Set UID)
- `sg`: 设置组ID位(Set GID)
- `ca`: 具有可执行位但没有用户ID或组ID位的文件(Capability): UNIX文件系统不支持
- `tw`: 目录, 粘滞位且其他人可写(Sticky and other Writable)
- `ow`: 目录, 无粘滞位且其他人可写(other Writable)
- `st`: 目录, 粘滞位(sticky bit)且其他人不可写
- `ex`: 可执行文件(Executable)

另一种是 unix [fnmatch](https://docs.python.org/3/library/fnmatch.html) 的正则匹配模式, 诸如此类的格式

```bash
*.tbz=01;31
*.tbz2=01;31
*.tz=01;31
*.deb=01;31
*.rpm=01;31
*.jar=01;31
```

> 正常来说这里需要实现一个 fnmatch 的正则解析器, 但为了简化实现暂时认为都是 `*.xxx` 的模式, **只考虑后缀**

因此在实现 ls 彩色输出的时候首先需要探查 LS_COLORS 环境变量并进行解析, 然后根据不同的文件类型以对应的虚拟终端序列输出

如果当前 shell 并没有 $LS_COLORS 变量, 那么 ls 就使用默认的内置颜色类型

> 比如使用 unset LS_COLORS 清除 LS_COLORS, 那么压缩文件等就不会显示红色了

笔者编写了一个小脚本用于创建各种文件类型并使用 ls 查看, 感兴趣的读者可与此处下载

```bash
wget https://raw.githubusercontent.com/luzhixing12345/coreutils/main/scripts/ls/ls_all_types_files.sh
./ls_all_types_files.sh
```

### 修改颜色

如果想要修改默认 ls 对于不同类型文件的颜色可以直接修改 LS_COLORS, 例如可以使用如下方式在当前 shell 中修改 LS_COLORS 补充添加对于目录 di 的颜色

```bash
export LS_COLORS="$LS_COLORS:di=01;31:"
```

![20230919083023](https://raw.githubusercontent.com/learner-lu/picbed/master/20230919083023.png)

修改 LS_COLORS 的另一个方式是通过 dircolors, 实际上 shell 启动时的 $LS_COLORS 就是 dircolors 来进行设置的, 具体信息请阅读下一部分 dircolors

## 日期显示

关于日期显示需要参考官方文档: [gnu Formatting-file-timestamps](https://www.gnu.org/software/coreutils/manual/html_node/Formatting-file-timestamps.html), 具体来说是六个月内没有被访问过的, 则显示年份, 否则显示具体时间

## 参考

- [ls command](https://wangchujiang.com/linux-command/c/ls.html)
- [quora How-do-I-understand-the-tmux-design-architecture-and-internals](https://www.quora.com/How-do-I-understand-the-tmux-design-architecture-and-internals)