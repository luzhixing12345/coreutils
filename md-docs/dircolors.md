
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

如果 /usr/bin/dircolors 文件存在, 则首先判断根目录下是否有 .dircolors, 如果存在则使用 .dircolors 作为配置文件, 否则使用 dircolors 的默认 bash 作为配置, 然后对于 ls grep 设置别名默认开启 --color=auto 的颜色显示

```bash
strace -e trace=open dircolors -b
```

## 参考

- [gnu dircolors-invocation](https://www.gnu.org/software/coreutils/manual/html_node/dircolors-invocation.html)
- [stackexchange unix](https://unix.stackexchange.com/questions/94299/dircolors-modify-color-settings-globaly)
- [die linux](https://linux.die.net/man/5/dir_colors)