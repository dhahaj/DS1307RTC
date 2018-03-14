#include <PrintEx.h>
#include <Wire.h>
#include "RTClib.h"

#define DELAY 4000

RTC_DS1307 rtc;
StreamEx s = Serial;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {

  Serial.begin(115200);

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }

  //rtc.adjust(DateTime(1521045046-18000));

  if (! rtc.isrunning()) {
    Serial.println(F("RTC is NOT running!"));
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(1521045046-18000));
  }
}

void loop () {
  DateTime now = rtc.now();
  printTime(&now);

  unsigned long start = millis();
  while (millis() - start < DELAY)
    if (Serial.available() > 3)
    {
      char c;
      String s = "";
      while ((c = (char)Serial.read()) != '\r')
        s += c;
      int val = s.toInt();
      if (val != 0)
        timeAdj(val);
    }
}

void printTime(DateTime* addr)
{
  DateTime now = *addr;

  s << now.month() << '/' << now.day() << '/' << now.year() << " (" << daysOfTheWeek[now.dayOfTheWeek()] << "), ";

  int h = now.hour();
  int m = now.minute();
  int sec = now.second();
  String ampm = (h > 11 ? " PM" : " AM");

  if (h <= 9)
    s << '0';
  if (h > 12)
    s << (h % 12) << ':';
  else
    s << h << ':';

  if (m <= 9)
    s << '0';
  s << m << ':';

  if (sec <= 9)
    s << '0';
  s << sec;

  s << ampm << ios::endl;

  unsigned long utime = now.unixtime();
  s << " UnixTime(1/1/1970) = " << utime << "sec = " << (utime / 86400L) << "days = ";
  s << (utime / 604800L) << "weeks = " << (utime / 31557600L) << "years" << ios::endl;
}

void timeAdj(int amount)
{
  DateTime now = rtc.now();
  rtc.adjust(now + amount);
  s << "Adjusted time by " << amount << " seconds." << ios::endl;
}


