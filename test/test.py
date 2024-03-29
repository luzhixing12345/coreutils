# see test/README.md for more information

import subprocess
import json

import difflib
import re


def print_diff(s1, s2):
    d = difflib.Differ()
    diff = list(d.compare(s1.splitlines(), s2.splitlines()))

    for line in diff:
        prefix = line[:2]
        text = line[2:]

        if prefix == "  ":
            print(f"  {text}")
        elif prefix == "- ":
            print(f"\033[91m- {text}\033[0m")
        elif prefix == "+ ":
            print(f"\033[92m+ {text}\033[0m")


def test_difference(command1, command2):
    # 使用subprocess运行命令,并捕获输出结果
    output1 = subprocess.run(command1, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    output2 = subprocess.run(command2, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    full_command = " ".join(command1)

    # 比较两个输出结果
    if output1.stdout == output2.stdout and output1.stderr == output2.stderr:
        print(f"  \033[92m[passed]: {full_command}\033[0m")
        return "passed"
    else:
        print(f"  \033[91m[failed]: {full_command}\033[0m")
        print(output1.stderr, output2.stderr)
        print_diff(output1.stdout, output2.stdout)


def main():
    with open("./test/test.json", "r", encoding="utf-8") as f:
        data = json.load(f)

    valgrind_pattern = re.compile(r"ERROR SUMMARY: (\d+) errors from (\d+) contexts")

    for program_name in data:
        print(f"testing {program_name}:")
        my_program_name = f"./src/{program_name}"

        files = data[program_name].get("files", [""])
        argruments = data[program_name].get("args", [""])

        case_number = 0
        passed_case_number = 0
        for file in files:
            for args in argruments:
                case_number += 1

                command1: str = program_name
                if file != "":
                    command1 += " " + file
                if args != "":
                    command1 += " " + args

                command2 = my_program_name
                if file != "":
                    command2 += " " + file
                if args != "":
                    command2 += " " + args

                # 与原 coreutils 的行为是否一致
                action_result = test_difference(command1.split(" "), command2.split(" "))

                # 利用 valgrind 判断是否存在内存泄漏
                valgrind_command = [
                    "valgrind",
                    "-s",
                    "--leak-check=full",
                    "--show-leak-kinds=all",
                    my_program_name,
                ]
                valgrind_output = subprocess.run(
                    valgrind_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
                )
                match_group = re.search(valgrind_pattern, valgrind_output.stderr)
                if match_group.group(1) != "0":
                    print(f"memory error: {match_group.group(1)}")

                if action_result == "passed":
                    passed_case_number += 1

        print(f"{program_name} passed [{passed_case_number}/{case_number}]")


if __name__ == "__main__":
    DEBUG = 0
    if not DEBUG:
        main()
    else:
        # test single instruction
        instruction = "readelf examples/a -s"
        command1 = instruction.split(" ")
        command2 = instruction.split(" ")
        command2[0] = f"./src/{command2[0]}"
        result = test_difference(command1, command2)
        if result == "passed":
            print("passed")
