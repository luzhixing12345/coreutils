import subprocess
import re

# 定义要运行的命令和valgrind参数
command = ['./src/ls']  # 将此处替换为你的程序路径和参数
valgrind_args = ['valgrind', '--leak-check=full'] + command

try:
    # 执行valgrind命令,将错误信息保存到valgrind.log文件
    result = subprocess.run(valgrind_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # 将标准错误信息保存到valgrind.log文件
    with open('valgrind.log', 'w') as log_file:
        log_file.write(result.stderr)
    
    valgrind_result_pattern = re.compile(r'ERROR SUMMARY: (\d+) errors from (\d+) contexts')
    match_result = valgrind_result_pattern.search(result.stderr)
    error = match_result.group(1)
    contexts = match_result.group(2)
    print(error, contexts)
except Exception as e:
    print("An error occurred:", e)
