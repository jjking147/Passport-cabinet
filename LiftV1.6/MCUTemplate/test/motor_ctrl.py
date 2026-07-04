"""
电机控制脚本 - 通过COM8直接控制SVD750-RS电机驱动
用法: python motor_ctrl.py COMx [命令] [参数]
命令:
  read          读取编码器和状态
  move <位置>   移动到绝对位置
  rel <步数>    相对移动
  stop          停止
  zero          回零（大正数向前走）
  init          初始化电机
示例:
  python motor_ctrl.py COM8 read
  python motor_ctrl.py COM8 move 100000
  python motor_ctrl.py COM8 rel 5000
  python motor_ctrl.py COM8 init
  python motor_ctrl.py COM8 move 100000 --speed 3000 --acc 10000
"""
import sys
import serial
import struct
import time
import argparse

MOTOR_ADDR = 0x03

def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc

def send_06(ser, reg, val):
    cmd = struct.pack('>BBHH', MOTOR_ADDR, 0x06, reg, val)
    cmd += struct.pack('<H', crc16(cmd))
    ser.write(cmd)
    time.sleep(0.05)
    resp = ser.read(ser.in_waiting)
    if resp and len(resp) >= 8:
        return True
    return False

def send_10(ser, reg, val32):
    cmd = struct.pack('>BBHH', MOTOR_ADDR, 0x10, reg, 2)
    cmd += bytes([4]) + struct.pack('>i', val32)
    cmd += struct.pack('<H', crc16(cmd))
    ser.write(cmd)
    time.sleep(0.05)
    resp = ser.read(ser.in_waiting)
    if resp and len(resp) >= 8:
        return True
    return False

def read_03(ser, reg, count):
    cmd = struct.pack('>BBHH', MOTOR_ADDR, 0x03, reg, count)
    cmd += struct.pack('<H', crc16(cmd))
    ser.write(cmd)
    time.sleep(0.1)
    resp = ser.read(ser.in_waiting)
    if resp and len(resp) >= 5 + count * 2:
        return resp
    return None

def read_u16(ser, reg):
    resp = read_03(ser, reg, 1)
    if resp:
        return struct.unpack('>H', resp[3:5])[0]
    return None

def read_s32(ser, reg):
    resp = read_03(ser, reg, 2)
    if resp:
        return struct.unpack('>i', resp[3:7])[0]
    return None

def init_motor(ser):
    print("Initializing motor...")
    send_06(ser, 0x0384, 0x0007)  # clear alarm
    time.sleep(0.3)
    send_06(ser, 0x0384, 0x0006)  # power on
    send_06(ser, 0x0384, 0x0007)  # init
    send_06(ser, 0x0384, 0x000F)  # enable
    print("Done")

def read_status(ser):
    print("=== Motor Status ===")
    status = read_u16(ser, 0x0064)
    arrived = read_u16(ser, 0x038B)
    pos = read_s32(ser, 0x0066)
    turn = read_s32(ser, 0x0068)
    print(f"  Status register (0x0064): {status} (0x{status:04X})" if status is not None else "  Status: read failed")
    print(f"  Arrived (0x038B):         {arrived}" if arrived is not None else "  Arrived: read failed")
    print(f"  Encoder (0x0066):         {pos}" if pos is not None else "  Encoder: read failed")
    print(f"  Turn (0x0068):            {turn}" if turn is not None else "  Turn: read failed")

def move_to(ser, target, speed=3000, acc=10000, dec=10000):
    print(f"Moving to {target} (speed={speed}, acc={acc}, dec={dec})...")
    init_motor(ser)
    send_10(ser, 0x038C, speed)
    send_10(ser, 0x0392, acc)
    send_10(ser, 0x0394, dec)
    send_10(ser, 0x0386, target)
    send_06(ser, 0x0385, 0x0001)
    print("Triggered. Monitoring...")
    for i in range(60):
        time.sleep(0.5)
        status = read_u16(ser, 0x0064)
        arrived = read_u16(ser, 0x038B)
        pos = read_s32(ser, 0x0066)
        err = read_u16(ser, 0x0064)
        if arrived == 1:
            print(f"  Arrived! position={pos}")
            send_06(ser, 0x0385, 0x0000)
            return True
        if status and status != 0:
            print(f"  Error! status={status} (0x{status:04X})")
            send_06(ser, 0x0385, 0x0000)
            return False
        if i % 4 == 0:
            print(f"  Moving... position={pos}, status={status}")
    print("  Timeout!")
    send_06(ser, 0x0385, 0x0000)
    return False

def move_rel(ser, steps, speed=3000, acc=10000, dec=10000):
    pos = read_s32(ser, 0x0066)
    if pos is None:
        print("Failed to read current position")
        return
    target = pos + steps
    print(f"Current: {pos}, target: {target} (delta={steps})")
    move_to(ser, target, speed, acc, dec)

def stop_motor(ser):
    print("Stopping...")
    send_06(ser, 0x0385, 0x0000)
    send_06(ser, 0x0384, 0x000B)
    print("Stopped")

def main():
    parser = argparse.ArgumentParser(description="Motor control")
    parser.add_argument("port", help="Serial port (e.g. COM8)")
    parser.add_argument("command", choices=["read", "move", "rel", "stop", "zero", "init"], help="Command")
    parser.add_argument("value", type=int, nargs="?", default=0, help="Position/steps value")
    parser.add_argument("--speed", type=int, default=3000, help="Speed (default 3000)")
    parser.add_argument("--acc", type=int, default=10000, help="Acceleration (default 10000)")
    parser.add_argument("--dec", type=int, default=10000, help="Deceleration (default 10000)")
    args = parser.parse_args()

    ser = serial.Serial(args.port, 115200, timeout=0.5)
    print(f"Opened {args.port}")

    if args.command == "read":
        read_status(ser)
    elif args.command == "init":
        init_motor(ser)
    elif args.command == "move":
        move_to(ser, args.value, args.speed, args.acc, args.dec)
    elif args.command == "rel":
        move_rel(ser, args.value, args.speed, args.acc, args.dec)
    elif args.command == "zero":
        move_to(ser, 0x0FFFFFFF, args.speed, args.acc, args.dec)
    elif args.command == "stop":
        stop_motor(ser)

    ser.close()

if __name__ == '__main__':
    main()
