# cd c:\python34
# scripts\pip.exe install pyserial

import  serial
from datetime import datetime
import time

dow = ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT']
ser = serial.Serial("COM4", 9600)
time.sleep(5)
utc = datetime.utcnow()
str = dow[utc.isoweekday()%7] + ' ' + utc.strftime ('%Y-%m-%d %H:%M:%S') + '\n'
ser.write(str.encode('utf-8'))
ser.flush()
print(str)
time.sleep(2)

