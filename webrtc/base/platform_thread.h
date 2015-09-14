/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_BASE_PLATFORM_THREAD_H_
#define WEBRTC_BASE_PLATFORM_THREAD_H_

#if defined(WEBRTC_WIN)
#include <winsock2.h>
#include <windows.h>
#elif defined(WEBRTC_POSIX)
#include <pthread.h>
#include <unistd.h>
#endif

namespace rtc {

#if defined(WEBRTC_WIN)
typedef DWORD PlatformThreadId;
typedef DWORD PlatformThreadRef;

// See https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;  // Must be 0x1000.
  LPCSTR szName;  // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
#elif defined(WEBRTC_POSIX)
typedef pid_t PlatformThreadId;
typedef pthread_t PlatformThreadRef;
#endif

PlatformThreadId CurrentThreadId();
PlatformThreadRef CurrentThreadRef();

// Compares two thread identifiers for equality.
bool IsThreadRefEqual(const PlatformThreadRef& a, const PlatformThreadRef& b);

// Sets the current thread name.
void SetCurrentThreadName(const char* name);
#if defined(WEBRTC_WIN)
void SetCurrentThreadNameHelper(THREADNAME_INFO threadname_info);
#endif  // WEBRTC_WIN

}  // namespace rtc

#endif  // WEBRTC_BASE_PLATFORM_THREAD_H_
