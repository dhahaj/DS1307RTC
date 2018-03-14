#include <PrintEx.h>
#include <Wire.h>
#include <stddef.h>
#include <Messenger.h>
#include "RTClib.h"

#define DELAY 4000
#define SEPARATOR ' '

RTC_DS1307 rtc;
StreamEx s = Serial;
Messenger message = Messenger(SEPARATOR);
char buffer[30];

#ifdef DEC
#undef DEC
#endif

const char JAN[] PROGMEM = "January", FEB[] PROGMEM = "Feburary", MAR[] PROGMEM = "March", APR[] PROGMEM = "April", MAY[] PROGMEM = "May", JUN[] PROGMEM = "June", JUL[] PROGMEM = "July",
                 AUG[] PROGMEM = "September", SEP[] PROGMEM = "October", NOV[] PROGMEM = "November", DEC[] PROGMEM = "December";
const char DAY1[] PROGMEM = "Sunday", DAY2[] PROGMEM = "Monday", DAY3[] PROGMEM = "Tuesday", DAY4[] PROGMEM = "Wednesday", DAY5[] PROGMEM = "Thursday", DAY6[] PROGMEM = "Friday", DAY7[] PROGMEM = "Saturday";

const char* const day_table[] PROGMEM = {DAY1, DAY2, DAY3, DAY4, DAY5, DAY6, DAY7};
const char* const month_table[] PROGMEM = {JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, NOV, DEC};

const char HELP_MSG[] PROGMEM = {"Commands:\r\n " \
                                 "t = print the time\r\n " \
                                 "d = print the date\r\n " \
                                 "u = print full unix time\r\n " \
                                 "s = adjust time in seconds\r\n " \
                                 "Y = set the year\r\n " \
                                 "M = set the month\r\n " \
                                 "D = set the day\r\n " \
                                 "m = adjust by this many minutes\r\n " \
                                 "h = adjust by this many hours\r\n " \
                                 "f = print full date and time\r\n"
                                };

void setup(void) {
  Serial.begin(115200);
  message.attach(messageCompleted);
  byte count = 0;
  while (1) {
    if (!rtc.begin()) {
      count++;
      s << "Couldn't find RTC... count=" << count << ios::endl;
      delay(500);
    } else if (count >= 3) {
      s << "Giving up!" << ios::endl;
      while (1);
    } else
      break;
  }

  if (! rtc.isrunning()) {
    s << F("RTC is NOT running!") << ios::endl;
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // or use this:
    // rtc.adjust(DateTime(2018, 3, 15, 11, 30, 30)); // 3/15/2018, 11:30:30AM
  }
}

void loop(void)
{
  while (Serial.available())
    message.process(Serial.read());
}

void messageCompleted(void)
{
  DateTime now = rtc.now();
  int i;
  char c = message.readChar();
  switch (c)
  {
    case 't':
      printTime();
      s << ios::endl;
      break;

    case 'd':
      printDate();
      s << ios::endl;
      break;

    case 'u':
      printUnixTime(&now);
      break;

    case 's':
      i = message.readInt();
      s << "Adjusting time by " << i << " seconds." << ios::endl;
      rtc.adjust(DateTime(now + i));
      break;

    case 'm':
      i = message.readInt();
      if (i > 0) {
        s.printf("Adjusting by %i minutes.%s", i, "\r\n");
        timeAdj(i * 60L);
      } else s << "Invalid entry: " << i << ios::endl;
      break;

    case 'h':
      i = message.readInt();
      s.printf("Adjusting by %i hours.%s", i, "\r\n");
      timeAdj(i * 3600UL);
      break;

    case 'M':
      i = message.readInt();
      if (i < 0 || i > 12) break;
      s.printf("Adjusting month to %i.%s", i, "\r\n");
      rtc.adjust(DateTime(now.year(), i, now.day(), now.hour(), now.minute(), now.second()));
      break;

    case 'D':
      i = message.readInt();
      if (i < 0 || i > 30) break;
      s.printf("Adjusting day to %i.%s", i, "\r\n");
      rtc.adjust(DateTime(now.year(), now.month(), i, now.hour(), now.minute(), now.second()));
      break;

    case 'y':
    case 'Y':
      i = message.readInt();
      s.printf("Adjusting year to %i.%s", i, "\r\n");
      rtc.adjust(DateTime(i, now.month(), i, now.hour(), now.minute(), now.second()));
      break;

    case 'f':
    case 'F':
      s << now.month() << '/' << now.day() << '/' << now.year() << ", ";
      printTime();
      s << ios::endl;
      break;

    default:
      s << F("Unknown command: ") << c << ios::endl;
      for (i = 0; i < strlen_P(HELP_MSG); i++)
        s << (char) pgm_read_byte_near(HELP_MSG + i);
      break;
  }
}

void printTime(void)
{
  DateTime now = rtc.now();

  uint8_t h = now.hour();
  uint8_t m = now.minute();
  uint8_t sec = now.second();
  String ampm = (h > 11 ? " PM" : " AM");
  if (h <= 9) s << '0';
  if (h > 12) s << (h % 12) << ':';
  else s << h << ':';
  if (m <= 9) s << '0';
  s << m << ':';
  if (sec <= 9) s << '0';
  s << sec;
  s << ampm;// << ios::endl;
}

void printDate(void)
{
  DateTime now = rtc.now();
  strcpy_P(buffer, (char*)pgm_read_word(&(day_table[now.dayOfTheWeek()])));
  s << buffer << ", ";
  strcpy_P(buffer, (char*)pgm_read_word(&(month_table[now.month() - 1])));
  s << buffer << ' '  << now.day() << " " << now.year(); // << ios::endl;
}

void printUnixTime(DateTime* ptr)
{
  DateTime now = rtc.now();
  unsigned long utime = now.unixtime();
  s << F("UnixTime (1/1/1970):") << ios::endl \
    << " -" << utime << F(" seconds") << ios::endl \
    << " -" << (utime / 60L) << F(" minutes") << ios::endl \
    << " -" << (utime / 3600L) << F(" hours") << ios::endl \
    << " -" << (utime / 86400L) << F(" days") << ios::endl \
    << " -" << (utime / 604800L) << F(" weeks") << ios::endl \
    << " -" << (utime / 31557600L) << F(" years") << ios::endl << ios::endl;
}

void timeAdj(int amount)
{
  DateTime now = rtc.now();
  rtc.adjust(now + amount);
  s << F("Adjusted time by ") << amount << F(" seconds.") << ios::endl;
}


