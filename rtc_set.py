#!/usr/bin/env python
import serial
from datetime import datetime, timezone
import time

dow = ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT']
ser = serial.Serial("/dev/ttyUSB0", 9600)   # COM6 on Windows
print("Sleeping for five seconds ...")
time.sleep(5)                               # Sleep cause Arduino is likely reseting now
utc = datetime.now(timezone.utc)
str = dow[utc.isoweekday()%7] + ' ' + utc.strftime ('%Y-%m-%d %H:%M:%S') + '\n'
print("Writing time; ", str);
ser.write(str.encode('utf-8'))              # Convert string to utf-8 (should be plain ascii)
ser.flush()
time.sleep(2)

