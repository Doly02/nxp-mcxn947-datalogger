#   Project:        NXP MCXN947 Datalogger
#   Author:         Tomas Dolak
#   File:           stress_test.py
#   Description:    This File Contains Stress Test of Digital Data Logger. 
#   

# Libraries
import serial
import time

# Configuration
PORT = "COM12"
BAUDRATE = 921600

def main():
    try:
        with serial.Serial(PORT, BAUDRATE, timeout=0) as ser:
            data = 0
            while True:
                msg = f"New New New New New New New New New New New New New New New New Data Data Data Data Data Data Data Data Data Data Data Data Data Data Data Data Data 0x{data:08X}\r\n"
                ser.write(msg.encode('ascii'))
                data = (data + 1) & 0xFFFFFFFF  # Overflow
                
    except serial.SerialException as e:
        print(f"ERR: Serial: {e}")
    except KeyboardInterrupt:
        print("INFO: Program Aborted")

if __name__ == "__main__":
    main()