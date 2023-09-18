#!/bin/bash

mkdir temp && cd temp

# 创建不同类型的文件
touch regular_file.txt
mkdir directory
ln -s regular_file.txt symlink
mkfifo fifo_pipe
touch executable_file
chmod +x executable_file

tar -czf files.tar.gz regular_file.txt directory symlink fifo_pipe executable_file


# 显示不同颜色的 ls 输出
ls -l --color=auto

cd .. && rm -r temp
