import random
import subprocess # 用于调用C++程序

# --- 配置参数 ---
NUM_TEST_CASES = 10000   # 测试用例数量
MAX_DIGITS = 200      # 生成大整数的最大位数 (可以根据你的BigInt能力调整)
CPP_EXECUTABLE_PATH = "bigInt.exe" # C++编译后的可执行文件路径

# --- 辅助函数 ---
def generate_big_int_str(max_digits):
    """生成一个大整数字符串"""
    if max_digits == 0:
        return "0"
    length = random.randint(1, max_digits)
    if length == 1 and random.random() < 0.1: # 有一定概率生成0
        return "0"

    digits = [str(random.randint(0, 9)) for _ in range(length)]
    if digits[0] == '0' and length > 1: # 避免前导零 (除非是单个0)
        digits[0] = str(random.randint(1, 9))

    s = "".join(digits)
    if random.random() < 0.4 and s != "0": # 约40%的概率为负数 (0不加负号)
        s = "-" + s
    return s

def get_python_result(num1_str, op_str, num2_str):
    """使用Python计算结果"""
    num1 = int(num1_str)
    num2 = int(num2_str)
    try:
        if op_str == '+':
            return str(num1 + num2)
        elif op_str == '-':
            return str(num1 - num2)
        elif op_str == '*':
            return str(num1 * num2)
        elif op_str == '/':
            if num2 == 0:
                return "ZeroDivisionError" # C++中通常会抛出异常或返回特定值
            # Python的 // 对于负数行为可能与C++的整数除法不同，要注意对齐
            # C++ a / b 向零取整
            # Python a // b 向下取整
            # 为了匹配C++的截断除法 (向零取整):
            if (num1 * num2 < 0) and (num1 % num2 != 0):
                 return str(num1 // num2 + 1) if num2 < 0 else str(num1 // num2 +1 if num1 <0 else num1 // num2) # 更复杂的处理
            # 简单处理：先用标准库的int，后续再精确对齐
            # 或者更精确地模拟C++的整数除法
            # result = num1 // num2
            # return str(result)
            # C++ style integer division (truncate towards zero)
            return str(int(num1 // num2))

        elif op_str == '%':
            if num2 == 0:
                return "ZeroDivisionError"
            # Python的 % 结果符号与除数相同
            # C++的 % 结果符号与被除数相同 (C++11及之后标准)
            # 为了匹配C++11之后的 % 行为：
            res = num1 % num2
            # 如果你的C++实现与Python的%行为不一致，需要在这里调整
            # 例如，如果Python是-7 % 5 = 3, C++可能是 -7 % 5 = -2
            # 如果要匹配C++的% (结果符号与被除数一致):
            # py_mod = num1 % num2
            # if py_mod != 0 and (num1 < 0) != (py_mod < 0) :
            #    py_mod += num2 #调整
            # return str(py_mod)
            # 假设 C++ 的 % 结果与被除数符号一致
            return str(num1 % num2)

    except ZeroDivisionError:
        return "ZeroDivisionError"
    return "Error" #不应该到这里

def get_second_num(choice):
    if choice == 1:
        num2_str = generate_big_int_str(random.randint(1, MAX_DIGITS))
    else:
        num2_str = str(random.randint(-2147483648, 2147483647))
    return num2_str

# --- 主测试逻辑 ---
def run_test():
    passed_count = 0
    failed_cases = []

    operations = ['+', '-', '*', '/'] # 初始可以先测试加减乘，除法和取模比较复杂
    # if MAX_DIGITS > 20: # 对于非常大的数，除法和取模可能非常耗时
    #     operations = ['+', '-', '*']


    for i in range(NUM_TEST_CASES):
        print(f"--- Test Case {i+1}/{NUM_TEST_CASES} ---")
        choice = random.randint(1, 2)
        num1_str = generate_big_int_str(MAX_DIGITS)
        num2_str = get_second_num(choice)
        op_str = random.choice(operations)
        # 特殊情况处理：避免除以0
        if op_str in ['/', '%'] and num2_str == "0":
            num2_str = get_second_num(choice)
            while num2_str == "0" or num2_str == "-0": # 确保不是0
                 num2_str = get_second_num(choice)


        # print(f"Python evaluating: {num1_str} {op_str} {num2_str}")
        python_expected_result = get_python_result(num1_str, op_str, num2_str)
        # print(f"Python result: {python_expected_result}")

        # 调用 C++ 程序
        try:
            # 通过 stdin 传递数据，通过 stdout 获取结果
            process = subprocess.Popen(
                [CPP_EXECUTABLE_PATH],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True # 使用文本模式
            )
            # 构造输入给C++: "num1 op num2\n"
            cpp_input = f"{choice}\n{num1_str}\n{op_str}\n{num2_str}\n"
            cpp_output, cpp_error = process.communicate(input=cpp_input, timeout=15) # 设置超时

            if process.returncode != 0 :
                 print(f"C++ program exited with error code: {process.returncode}")
                 print(f"C++ stderr: {cpp_error.strip()}")
                 failed_cases.append({
                    "input": f"{num1_str} {op_str} {num2_str}",
                    "python": python_expected_result,
                    "cpp": "Runtime Error or Crash",
                    "error": cpp_error.strip()
                })
                 continue


            cpp_actual_result = cpp_output.strip() # 去除可能的换行符
            # print(f"C++   result: {cpp_actual_result}")

            # 结果比较
            # 对于除零错误，需要统一标准
            if python_expected_result == "ZeroDivisionError":
                # 假设你的C++程序在这种情况下会输出特定错误信息或行为
                # 这里简单判断是否C++也认为是除零 (需要你的C++程序配合)
                # 例如，如果C++在这种情况下输出 "Error: Division by zero"
                if "division by zero" in cpp_actual_result.lower() or \
                   "zerodivisionerror" in cpp_actual_result.lower(): # C++输出的错误信息
                    print("Test PASSED (Division by zero handled)\n")
                    passed_count += 1
                else:
                    print("Test FAILED (Division by zero mismatch)\n")
                    failed_cases.append({
                        "input": f"{num1_str} {op_str} {num2_str}",
                        "python": python_expected_result,
                        "cpp": cpp_actual_result
                    })
            elif cpp_actual_result == python_expected_result:
                print("Test PASSED\n")
                passed_count += 1
            else:
                print("Test FAILED\n")
                failed_cases.append({
                    "input": f"{num1_str} {op_str} {num2_str}",
                    "python": python_expected_result,
                    "cpp": cpp_actual_result
                })

        except subprocess.TimeoutExpired:
            print("C++ program timed out.")
            failed_cases.append({
                "input": f"{num1_str} {op_str} {num2_str}",
                "python": python_expected_result,
                "cpp": "Timeout",
            })
        except FileNotFoundError:
            print(f"Error: C++ executable not found at {CPP_EXECUTABLE_PATH}")
            return # 无法继续测试
        except Exception as e:
            print(f"An error occurred: {e}")
            failed_cases.append({
                "input": f"{num1_str} {op_str} {num2_str}",
                "python": python_expected_result,
                "cpp": f"Python script error: {e}",
            })


    print(f"\n--- Test Summary ---")
    print(f"Total tests: {NUM_TEST_CASES}")
    print(f"Passed: {passed_count}")
    print(f"Failed: {len(failed_cases)}")

    if failed_cases:
        print("\n--- Failed Cases ---")
        for case in failed_cases:
            print(f"Input:    {case['input']}")
            print(f"Python:   {case['python']}")
            print(f"C++:      {case['cpp']}")
            if "error" in case:
                print(f"C++ Err:  {case['error']}")
            print("-" * 20)

if __name__ == "__main__":
    run_test()