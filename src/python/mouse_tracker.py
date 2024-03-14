import pyautogui
import time
import serial
from serial import Serial
from scipy.interpolate import interp1d

top_strip_map_x = interp1d([1919,-1920],[0,144])
top_strip_map_y = interp1d([0,1080],[0,255])

RATE = 1
command = "0x2E 0x09 \n"

# print serial ports
import serial.tools.list_ports
ports = serial.tools.list_ports.comports()
print(ports)
for port, desc, hwid in sorted(ports):
    print("{}: {} [{}]".format(port, desc, hwid))

def get_cursor_position():
    # Get the current mouse position
    x, y = pyautogui.position()
    return x, y

if __name__ == "__main__":
    # Get Cursor Position
    x_coord, y_coord = get_cursor_position()
        
    try:
        with serial.Serial(port="COM4", baudrate=115200, timeout=0.05) as ser:
            # ser.write(serial.to_bytes(command.encode()))
            while True:
                # print any received data
                if ser.in_waiting > 0:
                    try:
                        print(ser.readline())
                    except Exception as e:
                        pass

                # Get Cursor Position
                x_coord, y_coord = get_cursor_position()
                
                top_strip_x = int(top_strip_map_x(x_coord))
                top_strip_y = int(top_strip_map_y(y_coord))

                # print(f"Top Strip X: {top_strip_x} Y: {top_strip_y}")       

                ser.write(f"{top_strip_x}".encode('utf-8'))         
                time.sleep(RATE)

    except Exception as e:
        print(e)
        exit()
