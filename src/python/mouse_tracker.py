import pyautogui
import time
import serial
from serial import Serial

RATE = 0.1
command = "0x2E 0x09 \n"

# print serial ports
import serial.tools.list_ports
ports = serial.tools.list_ports.comports()
print(ports)
for port, desc, hwid in sorted(ports):
    print("{}: {} [{}]".format(port, desc, hwid))


# ser = Serial(port="COM4", baudrate=115200, timeout=1, dtr=True)

def send_command(command):
    ser.write(command.encode('utf-8'))
    time.sleep(1)
    ser.close()
    return "Command sent successfully"

def get_cursor_position():
    # Get the current mouse position
    x, y = pyautogui.position()
    return x, y

if __name__ == "__main__":

    last_pos = False

    # Get Cursor Position
    x_coord, y_coord = get_cursor_position()

    # Determine Monitor: x coord pos == mon 0

    # 1 == TRUE
    # 2 == FALSE
    if x_coord > 0: last_pos = True
    else: last_pos = False
        
    try:
        with serial.Serial(port="COM4", baudrate=115200, timeout=0.05, dsrdtr=True) as ser:
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

                # Determine Monitor: x coord pos == mon 0

                # currently 1 but was 2
                if x_coord > 0 and not last_pos: 
                    ser.write("1".encode('utf-8'))
                    last_pos = not last_pos
                
                # currently 2 but was 1
                elif x_coord <= 0 and last_pos: 
                    ser.write("2".encode('utf-8'))
                    last_pos = not last_pos 
                
                time.sleep(RATE)
    except Exception as e:
        print(e)
        exit()
