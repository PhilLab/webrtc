/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/system_wrappers/include/tick_util.h"

#include "webrtc/base/timeutils.h"

namespace webrtc {

int64_t TickTime::MillisecondTimestamp() {
  return TicksToMilliseconds(TickTime::Now().Ticks());


#ifdef WINRT
  static const uint64 kFileTimeToUnixTimeEpochOffset = 116444736000000000ULL;
  static const uint64 kNTPTimeToUnixTimeEpochOffset = 2208988800000L;
  LARGE_INTEGER TickTime::app_start_time_ = {};  // record app start time
  int64_t TickTime::time_since_os_start_ = -1;  // when app start,
                                                // the os ticks in ms
  int64_t TickTime::os_ticks_per_second_ = -1;
#endif

void TickTime::UseFakeClock(int64_t start_millisecond) {
  use_fake_clock_ = true;
  fake_ticks_ = MillisecondsToTicks(start_millisecond);
}

int64_t TickTime::MicrosecondTimestamp() {
  return TicksToMicroseconds(TickTime::Now().Ticks());
}

int64_t TickTime::MillisecondsToTicks(const int64_t ms) {
  return ms * rtc::kNumNanosecsPerMillisec;
}
}

int64_t TickTime::TicksToMicroseconds(const int64_t ticks) {
  return ticks / rtc::kNumNanosecsPerMicrosec;
}

#ifdef WINRT

//Warning, right now, the app_start_time_ and time_since_os_start_ are not protected with mutex.
//we only call this function to sync the clock of testing device with ntp when the app starts.
//if we want to call this function periodically in the runtime,then, suggest to use mutex 
void TickTime::SyncWithNtp(int64_t timeFromNtpServer /*in ms*/ ){

  TIME_ZONE_INFORMATION timeZone;
  GetTimeZoneInformation(&timeZone);
  int64_t timeZoneBias = timeZone.Bias * 60 * 1000;  // milliseconds

  app_start_time_.QuadPart = timeFromNtpServer - kNTPTimeToUnixTimeEpochOffset - timeZoneBias;

  //since we just update the app refernce time, need to update the reference point as well.

  LARGE_INTEGER qpcnt;
  QueryPerformanceCounter(&qpcnt);

  LARGE_INTEGER qpfreq;
  QueryPerformanceFrequency(&qpfreq);

  os_ticks_per_second_ = qpfreq.QuadPart;

  time_since_os_start_ = qpcnt.QuadPart * 1000 / os_ticks_per_second_;
}


inline void TickTime::InitializeAppStartTimestamp() {
  if (time_since_os_start_ != -1)  // already initialized
    return;

  TIME_ZONE_INFORMATION timeZone;
  GetTimeZoneInformation(&timeZone);
  int64_t timeZoneBias = timeZone.Bias * 60 * 1000;  // milliseconds

  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);  // this will give us system file in UTC format

  app_start_time_.HighPart = ft.dwHighDateTime;
  app_start_time_.LowPart = ft.dwLowDateTime;

  app_start_time_.QuadPart = (app_start_time_.QuadPart -
      kFileTimeToUnixTimeEpochOffset) / 10000 /* 100nanoSecond/10*1000 = ms*/
                              - timeZoneBias;

  LARGE_INTEGER qpcnt;
  QueryPerformanceCounter(&qpcnt);

  LARGE_INTEGER qpfreq;
  QueryPerformanceFrequency(&qpfreq);

  os_ticks_per_second_ = qpfreq.QuadPart;

  time_since_os_start_ = qpcnt.QuadPart * 1000 / os_ticks_per_second_;
}
#endif

// Gets the native system tick count, converted to nanoseconds.
int64_t TickTime::QueryOsForTicks() {
  return rtc::TimeNanos();
}

}  // namespace webrtc
