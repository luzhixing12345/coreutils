
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

## 搜索文件

## 颜色

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