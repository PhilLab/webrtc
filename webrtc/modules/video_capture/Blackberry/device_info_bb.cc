/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "device_info_bb.h"

#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


#include "ref_count.h"
#include "trace.h"


namespace webrtc
{
namespace videocapturemodule
{
VideoCaptureModule::DeviceInfo*
VideoCaptureImpl::CreateDeviceInfo(const int32_t id)
{
    videocapturemodule::DeviceInfoBB *deviceInfo =
                    new videocapturemodule::DeviceInfoBB(id);
    if (!deviceInfo)
    {
        deviceInfo = NULL;
    }

    return deviceInfo;
}

DeviceInfoBB::DeviceInfoBB(const int32_t id)
    : DeviceInfoImpl(id)
{
}

int32_t DeviceInfoBB::Init()
{
    return 0;
}

DeviceInfoBB::~DeviceInfoBB()
{
}

uint32_t DeviceInfoBB::NumberOfDevices()
{
    WEBRTC_TRACE(webrtc::kTraceApiCall, webrtc::kTraceVideoCapture, _id, "%s", __FUNCTION__);

    uint32_t count = 0;
    char device[20];

    return count;
}

int32_t DeviceInfoBB::GetDeviceName(
                                         uint32_t deviceNumber,
                                         char* deviceNameUTF8,
                                         uint32_t deviceNameLength,
                                         char* deviceUniqueIdUTF8,
                                         uint32_t deviceUniqueIdUTF8Length,
                                         char* /*productUniqueIdUTF8*/,
                                         uint32_t /*productUniqueIdUTF8Length*/)
{
    WEBRTC_TRACE(webrtc::kTraceApiCall, webrtc::kTraceVideoCapture, _id, "%s", __FUNCTION__);
    return 0;
}

int32_t DeviceInfoBB::CreateCapabilityMap(
                                          const char* deviceUniqueIdUTF8)
{
    return 0;
}

bool DeviceInfoBB::IsDeviceNameMatches(const char* name,
                                       const char* deviceUniqueIdUTF8)
{
    if (std::strncmp(deviceUniqueIdUTF8, name, std::strlen(name)) == 0)
            return true;
    return false;
}

int32_t DeviceInfoBB::FillCapabilityMap(int fd)
{
    return 0;
}

} // namespace videocapturemodule
} // namespace webrtc

