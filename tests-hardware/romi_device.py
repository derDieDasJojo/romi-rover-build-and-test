import time
import serial
import json
import math
        
class RomiDevice():

    def __init__(self, device): 
        print(f"Opening a serial link to {device}")
        self.driver = serial.Serial(device, 115200)
        time.sleep(2.0)
        self.debug = True
        
    def print_debug(self, s):
        if self.debug:
            print(s)
        
    def set_debug(self, value):
        self.debug = value
        
    def get_debug(self):
        return self.debug
        
    def send_command(self, s):
        for i in range(5):
            values = self.try_command(s)
            status_code = values[0]
            if status_code == 0:
                return values
            if status_code > 0:
                raise RuntimeError(values[0])
            if status_code < 0:
                print("Warning: Sending failed. Retrying.")
        raise RuntimeError("Sending failed")
            
    def try_command(self, s):
        command = "#" + s + ":xxxx\r\n"
        self.print_debug(f"Command: {s} -> {command}")
        self.driver.write(command.encode('ascii'))
        reply = self.read_reply()
        self.print_debug(f"Response: {reply}")
        return self.parse_reply(reply)
            
        
    def read_reply(self):
        while True:
            b = self.driver.readline()
            s = b.decode("ascii").rstrip()
            if s[0] == "#":
                if s[1] == "!":
                    print("Log: %s" % s)
                else:
                    break;
        return s
        
    def parse_reply(self, line):
        s = str(line)
        start = s.find("[")
        end = 1 + s.find("]")
        array_str = s[start:end]
        return_values = json.loads(array_str)
        return return_values
