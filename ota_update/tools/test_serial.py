#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
test_serial.py — 串口连通性自检 (定位 PA2/PA3 接线问题)

模式一 loopback (测 USB-TTL 转接板 + COM 号, 不碰 MCU):
    1. 把下载口那根 USB-TTL 从板子上拔下来
    2. 把它的 TX 和 RX 两根引脚短接 (跳线/镊子碰一起)
    3. python test_serial.py --port COM? --mode loopback
    -> 发送 256 字节并核对回显, 全对说明转接板和 COM 号没问题

模式二 listen (测 MCU PA2 TX 方向):
    1. 下载口 USB-TTL 正常接回 PA2(TX)/PA3(RX)
    2. 板子跑 Bootloader 停在 "Waiting YMODEM" (每 ~3 秒发一次 0x43 'C')
    3. python test_serial.py --port COM? --mode listen --seconds 8
    -> 看到 0x43('C') 说明 PA2 发送方向通了; 啥都没有说明没接对/COM 号错

依赖: pip install pyserial
"""

import argparse
import time
import sys

try:
    import serial
except ImportError:
    sys.exit("缺少 pyserial，请先执行: pip install pyserial")


def open_port(port, baud):
    try:
        return serial.Serial(port, baud, bytesize=8, parity='N', stopbits=1, timeout=0.5)
    except (serial.SerialException, PermissionError) as e:
        sys.exit(f"打开 {port} 失败: {e}\n（端口不存在 / 被串口助手占用 / 驱动未装）")


def mode_loopback(port, baud):
    ser = open_port(port, baud)
    test = bytes(range(256))          # 0x00..0xFF 共 256 字节
    ser.reset_input_buffer()
    ser.write(test)
    ser.flush()
    time.sleep(0.2)
    echo = ser.read(256)
    ser.close()

    if echo == test:
        print(f"[OK] {port} 回环成功: 256 字节全部原样返回")
        print("     转接板 + COM 号正常, TX/RX 短接有效")
    elif len(echo) == 0:
        print(f"[FAIL] {port} 没收到任何回显")
        print("     可能: ① TX/RX 没真正短接 ② 这个 COM 不是这根转接板")
        print("     ③ 转接板坏 ④ 串口被其它软件占用")
    else:
        print(f"[FAIL] {port} 回显不完整: 发了 256 只收到 {len(echo)} 字节")
        print(f"     首个不同点: 发 0x{test[len(echo)] if len(echo) < 256 else 0:02X}")
    return echo == test


def mode_listen(port, baud, seconds):
    ser = open_port(port, baud)
    counts = {}
    deadline = time.time() + seconds
    print(f"[*] 监听 {port} {seconds} 秒 ... (Ctrl+C 提前结束)")
    try:
        while time.time() < deadline:
            b = ser.read(1)
            if b:
                v = b[0]
                counts[v] = counts.get(v, 0) + 1
                ch = chr(v) if 32 <= v < 127 else '.'
                print(f"    收到 0x{v:02X}  '{ch}'")
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()

    total = sum(counts.values())
    print(f"[*] 共收到 {total} 字节")
    if total == 0:
        print("[FAIL] 什么都没收到")
        print("     可能: ① 这个 COM 不是下载口(USART1) ② PA2 没接到转接板 RX")
        print("     ③ Bootloader 没停在 Waiting YMODEM ④ TX/RX 接反")
        return False
    # Bootloader 等待时每 3 秒发一次 0x43 'C'
    if 0x43 in counts:
        print(f"[OK] 收到 {counts[0x43]} 个 'C'(0x43) -> PA2(TX) → 电脑 方向正常")
        return True
    else:
        print("[?] 有数据但不是 'C'(0x43)，说明这个口不像是 Bootloader 的下载口")
        return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', required=True, help='串口号, 如 COM7')
    ap.add_argument('--baud', type=int, default=9600)
    ap.add_argument('--mode', choices=['listen', 'loopback'], default='listen')
    ap.add_argument('--seconds', type=int, default=8, help='listen 模式监听秒数')
    args = ap.parse_args()

    if args.mode == 'loopback':
        ok = mode_loopback(args.port, args.baud)
    else:
        ok = mode_listen(args.port, args.baud, args.seconds)
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()