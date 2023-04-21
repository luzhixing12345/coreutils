

import os
import subprocess

def main():
    
    test_dir = 'testfiles'
    target_dir = 'src'
    exe_names = os.listdir(test_dir)
    
    for exe_name in exe_names:
        
        with open(os.path.join(test_dir,exe_name),'r',encoding='utf-8') as f:
            shell_cmds = f.read().split('\n')
        
        # 默认程序
        for shell_cmd in shell_cmds:
            with open('output.txt','w') as f:
                subprocess.call('ls',stdout=f)




if __name__ == "__main__":
    main()