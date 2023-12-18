from SerialHandler import SerialHandler
import threading
import time
from datetime import datetime
import json

PORT = "COM16"
BAUDRATE = 9600

serial_handler = SerialHandler(PORT, BAUDRATE)
status = "preheat"
filetime = datetime.now().isoweekday()
filename = "output.txt"

def listen_serial():
    while(1):
        global status
        
        c = input()
        
        if(c == "p"):
            status= "preheat"
        elif(c == "c"):
            status= "charge"
        elif(c == "1"):
            status= "first crack"
        elif(c == "2"):
            status= "second crack"
        elif(c == "l"):
            status= "light"
        elif(c == "m"):
            status= "medium"
        elif(c == "d"):
            status= "dark"
        elif(c == "q"):
            status= "finish"
            
if __name__ == "__main__":
    thread_serial_listen = threading.Thread(target=listen_serial)
    thread_serial_listen.start()

    while(1):
        try:
            t = datetime.now()
            data = serial_handler.read()
            data["time"] = str(t)
            data["status"] = str(status)

            # print(data)

            out = json.dumps(data)
            
            # print(filename)
            # break

            f = open(filename, "+a")
            f.write(out + '\n')
            f.close()
        except Exception as e:
            print(e)
        # time.sleep(1)
