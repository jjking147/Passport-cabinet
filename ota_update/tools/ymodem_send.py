#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ymodem_send.py — GD32F407 OTA 固件下发工具 (YMODEM CRC, 1K 块)

功能:
  1. 把 Keil 生成的 .hex (Intel HEX) 转成纯二进制 (间隙填充 0xFF, 只保留 App 区)
  2. 通过串口以 YMODEM 协议发送给 Bootloader

依赖:
  pip install pyserial

用法:
  python ymodem_send.py --port COM3 --baud 115200 --file App_A.hex

典型流程:
  1. 给目标板断电重启, 或让 App 串口发 'O' 进入 Bootloader
  2. 串口打印 "Waiting YMODEM (CRC, 1K) transfer..." 后运行本脚本
"""

import argparse
import struct
import sys
import time

try:
    import serial
except ImportError:
    sys.exit("缺少 pyserial, 请先执行: pip install pyserial")

SOH = 0x01
STX = 0x02
EOT = 0x04
ACK = 0x06
NAK = 0x15
CAN = 0x18
C = 0x43

PKT_128 = 128
PKT_1K = 1024


# ----------------------------------------------------------------------
# XMODEM CRC16 (多项式 0x1021, 初值 0, MSB 先行) —— 与固件 ota_crc16() 一致
# ----------------------------------------------------------------------
def crc16(data: bytes) -> int:
    crc = 0
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if (crc & 0x8000) else (crc << 1) & 0xFFFF
    return crc


# ----------------------------------------------------------------------
# Intel HEX 解析 -> bytes (地址间隙填充 0xFF)
# ----------------------------------------------------------------------
def parse_hex(path: str) -> (int, bytes):
    """返回 (起始地址, 数据 bytes); 支持跨多个 64KB 段(扩展线性地址记录)"""
    seg = {}
    base = 0
    start = None
    with open(path, 'rt', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if not line.startswith(':'):
                continue
            b = line[1:]
            count = int(b[0:2], 16)
            addr = int(b[2:6], 16)
            rectype = int(b[6:8], 16)
            data = bytes.fromhex(b[8:8 + count * 2])
            if rectype == 0x00:  # data record
                absaddr = base + addr
                if start is None or absaddr < start:
                    start = absaddr
                seg[absaddr] = data
            elif rectype == 0x01:  # EOF
                break
            elif rectype == 0x04:  # extended linear address (upper 16 bits)
                base = int.from_bytes(data, 'big') << 16
            elif rectype == 0x02:  # extended segment address (upper 16 bits, 16-byte granularity)
                base = int.from_bytes(data, 'big') << 4
            # 其余类型 (03/05 起始地址) 忽略
    if start is None:
        sys.exit(f"{path}: 无有效 HEX 数据")

    hi = max(a + len(d) for a, d in seg.items())
    buf = bytearray(b'\xFF' * (hi - start))
    for a, d in seg.items():
        off = a - start
        buf[off:off + len(d)] = d
    return start, bytes(buf)


def read_hex_or_bin(path: str) -> (int, bytes):
    with open(path, 'rb') as f:
        head = f.read(1)
    if head == b':':
        return parse_hex(path)
    data = open(path, 'rb').read()
    return 0x08010000, data  # 裸 bin 默认按 App_A 基地址, 仅供参考


# ----------------------------------------------------------------------
# YMODEM 发送端
# ----------------------------------------------------------------------
def read_byte(ser: serial.Serial, timeout: float, expect=None) -> int:
    ser.timeout = timeout
    b = ser.read(1)
    if not b:
        raise TimeoutError(f"等待字节超时 ({timeout}s)")
    v = b[0]
    if expect is not None and v != expect and v not in (NAK, CAN):
        raise RuntimeError(f"期望 0x{expect:02X}, 收到 0x{v:02X}")
    return v


def wait_byte(ser: serial.Serial, timeout: float, accept) -> int:
    """一直读到 accept 中的某个字节为止"""
    deadline = time.time() + timeout
    while time.time() < deadline:
        ser.timeout = max(0.05, deadline - time.time())
        b = ser.read(1)
        if b and b[0] in accept:
            return b[0]
    raise TimeoutError("等待应答超时")


def send_packet(ser: serial.Serial, seq: int, data: bytes) -> int:
    n = len(data)
    if n <= PKT_128:
        hdr = SOH
        block = PKT_128
    else:
        hdr = STX
        block = PKT_1K
    pad = b'\x1a' * (block - n) if n < block else b''
    payload = data + pad
    pkt = bytes([hdr, seq & 0xFF, (~seq) & 0xFF]) + payload
    c = crc16(payload)
    pkt += bytes([(c >> 8) & 0xFF, c & 0xFF])
    ser.write(pkt)
    ser.flush()
    return block


def ymodem_send(ser: serial.Serial, start: int, data: bytes) -> None:
    name = f"app_{start:08X}.bin".encode()
    print(f"[*] 发送 {len(data)} 字节, 起始地址 0x{start:08X}")

    # 1) 等待 'C'
    print("[1] 等待接收端 'C' ...")
    wait_byte(ser, 30.0, {C})

    # 2) block 0 (文件名 + 尺寸, 128 字节)
    block0 = name + b'\x00' + str(len(data)).encode() + b' '
    send_packet(ser, 0, block0)
    wait_byte(ser, 5.0, {ACK})
    print("[2] 文件名块已确认")

    # 3) 数据块 (接收端 ACK 完文件名块后会直接等数据, 无需二次 'C' 握手)
    seq = 1
    off = 0
    while off < len(data):
        chunk = data[off:off + PKT_1K]
        send_packet(ser, seq, chunk)
        try:
            wait_byte(ser, 5.0, {ACK})
        except TimeoutError:
            print(f"    -> 重发块 {seq}")
            send_packet(ser, seq, chunk)
            wait_byte(ser, 5.0, {ACK})
        off += PKT_1K
        seq = (seq + 1) & 0xFF
        if off % (PKT_1K * 16) == 0 or off >= len(data):
            print(f"    {off}/{len(data)}")
    print("[4] 数据块发送完毕")

    # 5) EOT 双重握手
    print("[5] 发送 EOT ...")
    ser.write(bytes([EOT])); ser.flush()
    wait_byte(ser, 5.0, {NAK})
    ser.write(bytes([EOT])); ser.flush()
    wait_byte(ser, 5.0, {ACK})

    # 6) 空结束块
    wait_byte(ser, 5.0, {C})
    send_packet(ser, 0, b'')
    wait_byte(ser, 5.0, {ACK})
    print("[6] 传输完成 (空结束块已确认)")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', required=True, help='串口号, 如 COM3')
    ap.add_argument('--baud', type=int, default=9600)
    ap.add_argument('--file', required=True, help='固件文件 (.hex 或 .bin)')
    ap.add_argument('--timeout', type=float, default=1.0, help='串口超时')
    args = ap.parse_args()

    start, data = read_hex_or_bin(args.file)
    if len(data) == 0:
        sys.exit("固件为空")

    ser = serial.Serial(args.port, args.baud, bytesize=8, parity='N',
                        stopbits=1, timeout=args.timeout)
    print(f"[*] 打开 {args.port} @ {args.baud}")
    try:
        ymodem_send(ser, start, data)
    except (TimeoutError, RuntimeError) as e:
        print(f"[!] 失败: {e}")
        sys.exit(1)
    finally:
        ser.close()


if __name__ == '__main__':
    main()