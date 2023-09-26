
# dircolors

dircolors 用于控制终端中 LS_COLORS 的值, 进一步控制 ls 的颜色显示

如果使用的是 ubuntu, 打开 .bashrc 即可看到有如下一段的配置信息

```bash
if [ -x /usr/bin/dircolors ]; then
    test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
    alias ls='ls --color=auto'
    #alias dir='dir --color=auto'
    #alias vdir='vdir --color=auto'

    alias grep='grep --color=auto'
    alias fgrep='fgrep --color=auto'
    alias egrep='egrep --color=auto'
fi

alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'
```

如果 /usr/bin/dircolors 文件存在, 则首先判断根目录下是否有 .dircolors, 如果存在则使用 .dircolors 作为配置文件, 否则使用 dircolors 的默认 bash 作为配置, **然后对于 ls grep 设置别名默认开启 --color=auto 的颜色显示**

> 这里我一直有点没有理解, 为什么 ls 的默认 color 选项是 `always`, 还需要在 bashrc 中手动 alias, 因为明显 auto 比 always 做的更好...

## dircolors.hin 文件解析

原版 coreutils 的解决办法是读取 dircolors.hin 的内容将 dircolors.c 中的 G_line 在编译前做批量替换, 将 dircolors.hin 的内容在编译前替换到文件中

笔者这里简化了这一步, 直接将 dircolors.hin 的内容复制并写死在 dircolors.c 中

然后就是依次读取字符串进行解析, 这个文法规则很简单, 就是字符串 + 空格, # 开头为注释

> 需要注意的是有一些字符串代表了特定的含义, 如下 18 个特殊的文件属性, 需要单独判断替换. 其余的以 `.` 开头的直接当作文件后缀名处理

```c
typedef struct {
    char *full_name;
    char *short_name;
    int name_length;
} dc_map;

const dc_map builtin_dc_map[18] = {{"RESET", "rs", 5},
                                   {"DIR", "di", 3},
                                   {"LINK", "ln", 4},
                                   {"MULTIHARDLINK", "mh", 13},
                                   {"FIFO", "pi", 4},
                                   {"SOCK", "so", 4},
                                   {"DOOR", "do", 4},
                                   {"BLK", "bd", 3},
                                   {"CHR", "cd", 3},
                                   {"ORPHAN", "or", 6},
                                   {"MISSING", "mi", 7},
                                   {"SETUID", "su", 6},
                                   {"SETGID", "sg", 6},
                                   {"CAPABILITY", "ca", 10},
                                   {"STICKY_OTHER_WRITABLE", "tw", 21},
                                   {"OTHER_WRITABLE", "ow", 14},
                                   {"STICKY", "st", 6},
                                   {"EXEC", "ex", 4}};
```

> 不过原版的 dircolors -b 参数可以带一个文件路径, 这里省略了...

## 参考

- [gnu dircolors-invocation](https://www.gnu.org/software/coreutils/manual/html_node/dircolors-invocation.html)
- [stackexchange unix](https://unix.stackexchange.com/questions/94299/dircolors-modify-color-settings-globaly)
- [die linux](https://linux.die.net/man/5/dir_colors)