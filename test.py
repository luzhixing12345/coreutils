



import os, copy




import subprocess




def main():
    
    test_dir = 'testfiles'
    target_dir = 'src'
    file_names = os.listdir(test_dir)
    exe_names = []
    for file_name in file_names:
        if not file_name.endswith('txt'):
            exe_names.append(file_name)
    
    total_info = []
    
    for exe_name in exe_names:
        
        test_file = os.path.join(test_dir,exe_name)
        with open(test_file,'r',encoding='utf-8') as f:
            shell_cmds = f.read().split('\n')
        
        # 默认程序
        for i, shell_cmd in enumerate(shell_cmds):
            if shell_cmd == '':
                continue
            cmd_list = shell_cmd.split(' ')
            
            cmd_list[0] = str(exe_name)
            complete_shell_cmd = shell_cmd.replace('@',exe_name)
            default_result = subprocess.run(cmd_list, stdout=subprocess.PIPE)
            
            busybox_cmd_list = copy.deepcopy(cmd_list)
            busybox_cmd_list[0] = "/home/kamilu/busybox-1.36.0/busybox"
            busybox_cmd_list.insert(1,exe_name)
            busybox_result = subprocess.run(busybox_cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            
            cmd_list[0] = os.path.join(target_dir,exe_name,exe_name)
            xbox_result = subprocess.run(cmd_list, stdout=subprocess.PIPE)
            if default_result.stdout.decode() != xbox_result.stdout.decode():
                if busybox_result.stdout.decode() == xbox_result.stdout.decode():
                    warning_info = f'[Warning]: GNU result is different from (busybox and xbox) in [{complete_shell_cmd}] in [{test_file}] line [{i}]'
                    print(warning_info)
                    total_info.append(warning_info)
                    continue
                error_info = f'[Error]: [{complete_shell_cmd}] in [{test_file}] line [{i}]'
                print(error_info)
                total_info.append(error_info)
                with open(os.path.join(test_dir,f'{exe_name}_{i}_default.txt'),'w') as f:
                    f.write(f'{error_info}\n\n')
                    f.write(default_result.stdout.decode())
                with open(os.path.join(test_dir,f'{exe_name}_{i}_busy_box.txt'),'w') as f:
                    f.write(f'{error_info}\n\n')
                    f.write(busybox_result.stdout.decode())
                with open(os.path.join(test_dir,f'{exe_name}_{i}_xbox.txt'),'w') as f:
                    f.write(f'{error_info}\n\n')
                    f.write(xbox_result.stdout.decode())
            else:
                print(f'[Pass ]: [{complete_shell_cmd}]')

        
    if len(total_info) == 0:
        print("\nfinish test\n")

    else:
        print("\n\n")
        print('-'*10)
        print("Total Info:\n")
        for error_info in total_info:
            print(error_info)
        print('-'*10)

if __name__ == "__main__":
    main()




