#!/bin/bash

mkdir temp && cd temp

# 创建不同类型的文件
touch file
mkdir directory
ln -s file symlink
mkfifo fifo_pipe
touch executable_file
chmod +x executable_file
touch su_file
chmod u+s su_file
touch sg_file
chmod g+s sg_file

mkdir tw_dir
chmod +t,o+w tw_dir
mkdir ow_dir
chmod o+w ow_dir
mkdir st_dir
chmod +t st_dir

touch a.lz
touch a.gif
touch a.mkv
touch a.aac

tar -czf files.tar.gz file directory symlink fifo_pipe executable_file


# 显示不同颜色的 ls 输出
ls -l --color=auto
# ../src/ls -l --color=always

cd .. && rm -r temp
