import  serial
from datetime import datetime
import time

dow = ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT']
ser = serial.Serial("COM6", 9600)
time.sleep(5)                      # Sleep cause Arduino is likely reseting now
utc = datetime.utcnow()
str = dow[utc.isoweekday()%7] + ' ' + utc.strftime ('%Y-%m-%d %H:%M:%S') + '\n'
ser.write(str.encode('utf-8'))     # Convert string to utf-8 (should be plain ascii)
ser.flush()
print(str)
time.sleep(2)