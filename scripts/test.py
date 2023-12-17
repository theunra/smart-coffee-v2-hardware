from SerialHandler import SerialHandler
import threading
import time
from datetime import datetime
import json

PORT = "COM15"
BAUDRATE = 9600

serial_handler = SerialHandler(PORT, BAUDRATE)

def listen_serial():
    while(1):
        print("hello")
        data = serial_handler.read()
        time.sleep(0.05)

if __name__ == "__main__":
    # thread_serial_listen = threading.Thread(target=listen_serial)
    # thread_serial_listen.start()

    while(1):
        try:
            t = datetime.now()
            data = serial_handler.read()
            data["time"] = str(t)

            print(data)

            out = json.dumps(data)

            f = open("output.txt", "+a")
            f.write(out + '\n')
            f.close()
        except:
            pass
        # time.sleep(1)
