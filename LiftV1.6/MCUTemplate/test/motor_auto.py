#例程代码！！！！！！！！！！！！！！！！！！！
# """
# SVD750-RS Modbus 位置模式控制脚本
# 相对位移：当前位置 + N 圈
# Pn023=3, Pn001=10000, Pn026=3
# """

# import serial
# import time
# import struct
# import argparse

# MOTOR_ADDR = 0x03
# PULSES_PER_REV = 10000

# def crc16(data: bytes) -> int:
#     crc = 0xFFFF
#     for b in data:
#         crc ^= b
#         for _ in range(8):
#             crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
#     return crc

# def write_u16(ser, reg, val, verbose=True):
#     cmd = struct.pack('>BBHH', MOTOR_ADDR, 0x06, reg, val)
#     cmd += struct.pack('<H', crc16(cmd))
#     ser.write(cmd)
#     time.sleep(0.05)
#     resp = ser.read(8)
#     if verbose:
#         print(f"  0x{reg:04X}=0x{val:04X} -> {'OK' if len(resp)==8 else 'FAIL'}")
#     return len(resp) == 8

# def write_i32(ser, base_reg, value, verbose=True):
#     """写 int32 到 两个 uint16 寄存器，低字 base_reg，高字 base_reg+1"""
#     low = value & 0xFFFF
#     high = (value >> 16) & 0xFFFF
#     write_u16(ser, base_reg, low, verbose)
#     write_u16(ser, base_reg + 1, high, verbose)

# def read_u16(ser, reg):
#     cmd = struct.pack('>BBHH', MOTOR_ADDR, 0x03, reg, 1)
#     cmd += struct.pack('<H', crc16(cmd))
#     ser.write(cmd)
#     time.sleep(0.1)
#     resp = ser.read(7)
#     if len(resp) == 7:
#         return struct.unpack('>H', resp[3:5])[0]
#     return None

# def main():
#     parser = argparse.ArgumentParser()
#     parser.add_argument("port", help="COM port e.g. COM8")
#     parser.add_argument("--rev", type=int, default=100, help="Revolutions +/-")
#     parser.add_argument("--speed", type=int, default=200000)
#     parser.add_argument("--acc", type=int, default=500000)
#     parser.add_argument("--dec", type=int, default=500000)
#     args = parser.parse_args()

#     ser = serial.Serial(args.port, 115200, timeout=0.5)
#     print(f"Opened {args.port}")

#     delta = args.rev * PULSES_PER_REV
#     print(f"Relative move: {delta} pulses ({args.rev} rev)")

#     # ---- 参数写入 ----
#     print("Set speed...")
#     write_i32(ser, 0x038C, args.speed)

#     print("Set acc...")
#     write_i32(ser, 0x0392, args.acc)

#     print("Set dec...")
#     write_i32(ser, 0x0394, args.dec)

#     print("Set relative position...")
#     write_i32(ser, 0x0386, delta)

#     # ---- 触发 0→1 ----
#     print("Trigger start...")
#     write_u16(ser, 0x0385, 0x0000)
#     time.sleep(0.02)
#     write_u16(ser, 0x0385, 0x0001)

#     # ---- 等待到位 ----
#     print("Waiting for in-position...")
#     for i in range(120):
#         time.sleep(0.5)
#         if i % 4 == 0:
#             arrived = read_u16(ser, 0x038B)
#             print(f"  {i*0.5:.1f}s arrived={arrived}")
#         if read_u16(ser, 0x038B) == 1:
#             print(">>> Arrived!")
#             break

#     ser.close()
#     print("Done")

# if __name__ == "__main__":
#     main()


"""
SVD750-RS 相对位置运动
Pn001 = 1000（每转脉冲数）
Pn023 = 3（RS485 位置模式）
站号 = 3
"""

import serial
import time
import struct

MOTOR_ADDR = 0x03
PULSES_PER_REV = 1000   # ← 你指定的 Pn001

# -------------------- CRC16 --------------------
def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc

# -------------------- 写 16bit 寄存器 0x06 --------------------
def write_u16(ser, reg, val, verbose=True):
    cmd = struct.pack('>BBHH', MOTOR_ADDR, 0x06, reg, val)
    cmd += struct.pack('<H', crc16(cmd))
    ser.write(cmd)
    time.sleep(0.05)
    resp = ser.read(8)
    if verbose:
        print(f"  0x{reg:04X}=0x{val:04X} -> {'OK' if len(resp)==8 else 'FAIL'}")
    return len(resp) == 8

# -------------------- 写 32bit 相对位置 / 速度 / 加减速 --------------------
def write_i32(ser, base_reg, value, verbose=True):
    low = value & 0xFFFF
    high = (value >> 16) & 0xFFFF
    write_u16(ser, base_reg, low, verbose)
    write_u16(ser, base_reg + 1, high, verbose)

# -------------------- 读 16bit --------------------
def read_u16(ser, reg):
    cmd = struct.pack('>BBHH', MOTOR_ADDR, 0x03, reg, 1)
    cmd += struct.pack('<H', crc16(cmd))
    ser.write(cmd)
    time.sleep(0.1)
    resp = ser.read(7)
    if len(resp) == 7:
        return struct.unpack('>H', resp[3:5])[0]
    return None

# -------------------- 等待到位 --------------------
def wait_arrived(ser, timeout=30.0, verbose=True):
    t = 0.0
    while t < timeout:
        time.sleep(0.1)
        t += 0.1
        if read_u16(ser, 0x038B) == 1:
            if verbose:
                print(f"  >>> 到位（{t:.1f}s）")
            return True
    if verbose:
        print("  >>> 超时未到位")
    return False

# ===================== MAIN =====================
def main():
    ser = serial.Serial("COM8", 115200, timeout=0.5)
    print("Opened COM8\n")

    # ---- 参数设置 ----
    print("设置参数...")
    write_i32(ser, 0x038C, 20000)     # 速度 20000 pps
    write_i32(ser, 0x0392, 15000)     # 加速度
    write_i32(ser, 0x0394, 150000)    # 减速度

    # ---- 相对位移 140000 脉冲（Pn001=1000 → 140 圈） ----
    print("设置相对位移 +140000 脉冲...")
    write_i32(ser, 0x0386, 140000)

    # ---- 触发 0 → 1 ----
    print("触发启动...")
    write_u16(ser, 0x0385, 0x0000)
    time.sleep(0.02)
    write_u16(ser, 0x0385, 0x0001)

    # ---- 等待到位 ----
    wait_arrived(ser, timeout=60)

    ser.close()
    print("\n✅ 完成")

if __name__ == "__main__":
    main()