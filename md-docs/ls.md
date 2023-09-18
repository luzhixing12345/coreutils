
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
printf "\033[1;91m123\033[1;0m\n"
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

ls 对应的各个文件类型的颜色并不是写死的, 实际上我们可以看到不同的 shell, terminal 都会有一些预制的颜色主题

```bash
echo $LS_COLORS | tr ':' '\n'
```

rs=0: 重置所有属性
di=01;34: 目录(颜色为深蓝)
ln=01;36: 符号链接(颜色为深青)
mh=00: 多硬链接文件
pi=40;33: FIFO(颜色为黄色底部)
so=01;35: 套接字(颜色为深紫)
do=01;35: 目录(颜色为深紫)
bd=40;33;01: 块设备文件(颜色为黄色底部,粗体)
cd=40;33;01: 字符设备文件(颜色为黄色底部,粗体)
or=40;31;01: "其他可读"(Other Readable),用于表示其他用户有读取权限的文
mi=00: 可打印字符
su=37;41: 带有 setuid 位的文件(颜色为白色字体,红色底部)
sg=30;43: 带有 setgid 位的文件(颜色为黑色字体,黄色底部)
ca=30;41: 带有 capablities 的文件(颜色为黑色字体,红色底部)
tw=30;42: 可交换文件
ow=34;42: 其他可写目录
st=37;44: 套接字目录
ex=01;32: 可执行文件(颜色为深绿,粗体)

## 参考

- [ls command](https://wangchujiang.com/linux-command/c/ls.html)
- [quora How-do-I-understand-the-tmux-design-architecture-and-internals](https://www.quora.com/How-do-I-understand-the-tmux-design-architecture-and-internals)