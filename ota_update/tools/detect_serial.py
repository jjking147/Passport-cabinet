#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
detect_serial.py — 自动探测 MCU 下载口(USART1/PA2/PA3) 的真实波特率

原理:
  MCU 现在跑着回显测试: PA3 收到什么, 就从 PA2 和 PA9 各发回一份。
  于是只要用"正确的波特率"发 "hello", 就能原样收到 "hello"。
  挨个试常见波特率, 找到能原样回显的那个, 就是 MCU 下载口的真实波特率。

用法 (先关掉占用该串口的串口助手):
  python detect_serial.py --port COM7

判定:
  匹配到 115200  -> MCU 正常, 是你电脑串口助手波特率设错了
  匹配到其它值  -> MCU 下载口波特率配置错了 (时钟/分频问题)
  全都不匹配    -> 串口没接对 / MCU 没在跑回显程序 / 串口被占用
"""

import argparse
import time
import sys

try:
    import serial
except ImportError:
    sys.exit("缺少 pyserial，请先执行: pip install pyserial")

TEST = b"hello"
BAUDS = [2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 74880,
         115200, 128000, 230400, 256000, 460800, 921600]


def try_baud(port, baud):
    try:
        ser = serial.Serial(port, baud, bytesize=8, parity='N', stopbits=1,
                            timeout=0.4, rtscts=False)
    except (serial.SerialException, PermissionError) as e:
        print(f"  打开 {port}@{baud} 失败: {e}")
        return None  # 无法打开 (占用/不存在)
    # 清空接收缓冲, 发几次测试串, 读回
    for _ in range(3):
        ser.reset_input_buffer()
        ser.write(TEST)
        time.sleep(0.25)
    echo = ser.read(3 * len(TEST))
    ser.close()
    return echo


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', required=True, help='下载口串口号, 如 COM7')
    args = ap.parse_args()

    print(f"[*] 探测 {args.port} 的波特率 (发送 '{TEST.decode()}') ...")

    # 先确认这个口能打开
    try:
        s = serial.Serial(args.port, 115200, timeout=0.3)
        s.close()
    except (serial.SerialException, PermissionError) as e:
        sys.exit(f"[FAIL] 无法打开 {args.port}: {e}\n"
                 "       串口可能被串口助手占用 —— 请先关闭 COM5/COM7 上所有串口助手再运行。")

    matched = None
    for baud in BAUDS:
        echo = try_baud(args.port, baud)
        if echo is None:
            continue
        # 只要回显里能连续找到 "hello" 就算命中
        if TEST in echo:
            print(f"  {baud:>8} : 收到 {echo!r}  <== 匹配!")
            matched = baud
        else:
            print(f"  {baud:>8} : 收到 {echo[:20]!r}")

    print()
    if matched == 9600:
        print("[结论] 下载口 = 9600 正常。你电脑串口助手的波特率设错成别的了，改成 9600-8-N-1 即可。")
    elif matched:
        print(f"[结论] 下载口真实波特率 = {matched}，与程序里的 9600 不符 —— 是固件时钟/波特率配置问题。")
    else:
        print("[结论] 没有任何波特率能回显 'hello'。请检查 PA2/PA3 接线、串口占用、或 MCU 是否在跑回显程序。")
    sys.exit(0 if matched == 9600 else 1)


if __name__ == '__main__':
    main()