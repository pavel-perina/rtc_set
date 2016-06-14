#include <Wire.h>

#define DS3231_I2C_ADDRESS 0x68

char linebuf[80];

static bool readLine()
{
  byte p;
  for (p = 0; p < 80; p++) {
    while(!Serial.available())
      delay(10);
    linebuf[p] = Serial.read();
    if (linebuf[p] == '\n') {
      linebuf[p] = 0;
      return true;
    }
  }  
  return false;
}


static uint8_t bcd2bin (uint8_t val) 
{ 
  return val - 6 * (val >> 4); 
}

static bool verifyTime(
  byte dow,
  byte bcdYear,
  byte bcdMon,
  byte bcdDay,
  byte bcdHour,
  byte bcdMin,
  byte bcdSec
  )
{
  byte Y = bcd2bin(bcdYear), M = bcd2bin(bcdMon), D = bcd2bin(bcdDay),
       h = bcd2bin(bcdHour), m = bcd2bin(bcdMin), s = bcd2bin(bcdSec);
  //Serial.println("20%02d-%d-%d %d:%d:%d", Y, M, D, h, m, s);
  Serial.print("Y"); Serial.print(Y);
  Serial.print("M"); Serial.print(M);
  Serial.print("D"); Serial.print(D);
  Serial.print("h"); Serial.print(h);
  Serial.print("m"); Serial.print(m);
  Serial.print("s"); Serial.print(s);
  Serial.println();
  
  if (dow > 7 || Y > 99 || M > 12 || D > 31 || h > 23 || m > 59 || s > 59) 
    return false;
  return true;    
}
  
static void rtcSetTime(
  byte dow,
  byte bcdYear,
  byte bcdMon,
  byte bcdDay,
  byte bcdHour,
  byte bcdMin,
  byte bcdSec
  )
{
  Wire.begin(DS3231_I2C_ADDRESS);
  // Set date
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write((byte) 0);   // set address to 0 (seconds)
  Wire.write(bcdSec);
  Wire.write(bcdMin);
  Wire.write(bcdHour);
  Wire.write(dow);
  Wire.write(bcdDay);
  Wire.write(bcdMon);
  Wire.write(bcdYear);
  Wire.endTransmission();  

  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.endTransmission();  
  
}


static void rtcDisableAlarm()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write((byte) 0x0E);
  Wire.write(0b00000100);     // 0E INTCN pin in alarm mode, no alarms set
  Wire.write(0b00000000);     // 0F Reset current alarm(s)
  Wire.endTransmission();
}


char * rtcDateTimeStr()
{
  //                        0123456789012345678
  static char datetime[] = "20YY-MM-DDThh:mm:ssZ\0";
  byte bcd;
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);                              // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);    // request seven bytes of data from DS3231 starting from register 00h
  // second, minute, hour
  bcd = Wire.read() & 0x7f; // 00 seconds
  datetime[17] = '0' + (bcd>>4);
  datetime[18] = '0' + (bcd&0x0f);
  bcd = Wire.read();        // 01 minutes
  datetime[14] = '0' + (bcd>>4);
  datetime[15] = '0' + (bcd&0x0f);
  bcd = Wire.read() & 0x3f; // 02 hours (bit 7 is 12/24, bit 6 is AM/PM
  datetime[11] = '0' + (bcd>>4);
  datetime[12] = '0' + (bcd&0x0f);
  bcd = Wire.read();        // 03 day of week
  bcd = Wire.read();        // 04 date
  datetime[ 8] = '0' + (bcd>>4);
  datetime[ 9] = '0' + (bcd&0x0f);
  bcd = Wire.read() & 0x7f; // 05 month
  datetime[ 5] = '0' + (bcd>>4);
  datetime[ 6] = '0' + (bcd&0x0f);
  bcd = Wire.read();        // 06 year
  datetime[ 2] = '0' + (bcd>>4);
  datetime[ 3] = '0' + (bcd&0x0f);
  return datetime;
}

void setup() 
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Wire.begin();
  Serial.println("Clearing interrupt and disabling alarm ...");
  rtcDisableAlarm();
  Serial.print ("Current time is : ");
  Serial.println(rtcDateTimeStr());    
  //                                     00000000001111111111222
  //                                     01234567890123456789012  
  Serial.println("Enter date and time in DOW-YYYY-MM-DD-hh-mm-ss format.");
  Serial.println("DOW is SUN, MON, TUE, WED, THU, FRI, SAT");
  Serial.println("Hyphens are ignored, by the way ...");
  
}


void loop() 
{
  if (!readLine()) {
    Serial.print("Line read error (line too long?");
    return;
  }
  Serial.print("Input: ");
  Serial.println(linebuf);

  // NOTE: everything is in BCD format, year is without century
  byte dow = 255, year = 255, month = 255, day = 255, hour = 255, minute = 255, second = 255;

  // Parse day of weak;
  switch(linebuf[0]) {
  case 'S':
  case 's':
    if ((linebuf[1] == 'U') || (linebuf[1] == 'u'))
      dow = 0;
    if ((linebuf[1] == 'A') || (linebuf[1] == 'a'))
      dow = 6;
    break;
  case 'M':
  case 'm':
    dow = 1;
    break;
  case 'T':
  case 't':
    if ((linebuf[1] == 'U') || (linebuf[1] == 'u'))
      dow = 2;
    if ((linebuf[1] == 'H') || (linebuf[1] == 'h'))
      dow = 4;
    break;
  case 'W':
  case 'w':
    dow = 3;
    break;    
  case 'F':
  case 'f':
    dow = 5;
    break;      
  }
  
  year   = (linebuf[ 6]-'0')*16 + (linebuf[ 7]-'0');
  month  = (linebuf[ 9]-'0')*16 + (linebuf[10]-'0');
  day    = (linebuf[12]-'0')*16 + (linebuf[13]-'0');
  hour   = (linebuf[15]-'0')*16 + (linebuf[16]-'0');
  minute = (linebuf[18]-'0')*16 + (linebuf[19]-'0');
  second = (linebuf[21]-'0')*16 + (linebuf[22]-'0');

  // verify time
  bool result = verifyTime(dow, year, month, day, hour, minute, second);
  
  // set time
  if (result) {
    Serial.println("Setting time ...");
    rtcSetTime(dow, year, month, day, hour, minute, second);
  }
  
  Serial.print ("Current time is : ");
  Serial.println(rtcDateTimeStr());    
  
}

