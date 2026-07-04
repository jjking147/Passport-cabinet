"""RS485总线监听 - 实时输出"""
import sys
import serial
import time

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else "COM8"
    ser = serial.Serial(port, 115200, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, timeout=0.05)
    print(f"sniffing on {port}...", flush=True)

    buf = bytearray()
    last_time = time.time()

    while True:
        data = ser.read(256)
        if data:
            buf.extend(data)
            last_time = time.time()
        else:
            if buf and (time.time() - last_time > 0.05):
                hex_str = ' '.join(f'{b:02X}' for b in buf)
                print(f"len={len(buf):3d}: {hex_str}", flush=True)
                buf.clear()

if __name__ == '__main__':
    main()
