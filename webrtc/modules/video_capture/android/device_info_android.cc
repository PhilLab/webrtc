/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/video_capture/android/device_info_android.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include "webrtc/modules/video_capture/android/video_capture_android.h"
#include "webrtc/system_wrappers/interface/logging.h"
#include "webrtc/system_wrappers/interface/ref_count.h"
#include "webrtc/system_wrappers/interface/trace.h"

namespace webrtc
{

namespace videocapturemodule
{

static std::string ResolutionsToString(
    const std::vector<std::pair<int, int> >& pairs) {
  std::stringstream stream;
  for (size_t i = 0; i < pairs.size(); ++i) {
    if (i > 0)
      stream << ", ";
    stream << "(" << pairs[i].first << "x" << pairs[i].second << ")";
  }
  return stream.str();
}

struct AndroidCameraInfo {
  std::string name;
  int min_mfps, max_mfps;  // FPS*1000.
  bool front_facing;
  int orientation;
  std::vector<std::pair<int, int> > resolutions;  // Pairs are: (width,height).

  std::string ToString() {
    std::stringstream stream;
    stream << "Name: [" << name << "], mfps: [" << min_mfps << ":" << max_mfps
           << "], front_facing: " << front_facing
           << ", orientation: " << orientation << ", resolutions: ["
           << ResolutionsToString(resolutions) << "]";
    return stream.str();
  }
};

// Camera info; populated during DeviceInfoAndroid::Initialize() and immutable
// thereafter.
static std::vector<AndroidCameraInfo>* g_camera_info = NULL;

// Set |*index| to the index of |name| in g_camera_info or return false if no
// match found.
static bool FindCameraIndexByName(const std::string& name, size_t* index) {
  for (size_t i = 0; i < g_camera_info->size(); ++i) {
    if (g_camera_info->at(i).name == name) {
      *index = i;
      return true;
    }
  }
  return false;
}

// Returns a pointer to the named member of g_camera_info, or NULL if no match
// is found.
static AndroidCameraInfo* FindCameraInfoByName(const std::string& name) {
  size_t index = 0;
  if (FindCameraIndexByName(name, &index))
    return &g_camera_info->at(index);
  return NULL;
}

// static
void DeviceInfoAndroid::Initialize(JNIEnv* jni) {
  // TODO(henrike): this "if" would make a lot more sense as an assert, but
  // Java_org_webrtc_videoengineapp_ViEAndroidJavaAPI_GetVideoEngine() and
  // Java_org_webrtc_videoengineapp_ViEAndroidJavaAPI_Terminate() conspire to
  // prevent this.  Once that code is made to only
  // VideoEngine::SetAndroidObjects() once per process, this can turn into an
  // assert.
  if (g_camera_info)
    return;

  g_camera_info = new std::vector<AndroidCameraInfo>();
  jclass j_info_class =
      jni->FindClass("org/webrtc/videoengine/VideoCaptureDeviceInfoAndroid");
  jclass j_device_info_class =
      jni->FindClass("org/webrtc/videoengine/VideoCaptureDeviceInfoAndroid$DeviceInfo");
  jclass j_size_class =
      jni->FindClass("org/webrtc/videoengine/VideoCaptureDeviceInfoAndroid$FrameSize");
  assert(j_info_class);
  assert(j_device_info_class);
  assert(j_size_class);
  jmethodID j_device_info_array_method = jni->GetStaticMethodID(
      j_info_class, "getDeviceInfoArray", "()[Lorg/webrtc/videoengine/VideoCaptureDeviceInfoAndroid$DeviceInfo;");
  
  jobjectArray j_device_info_array = static_cast<jobjectArray>(
        jni->CallStaticObjectMethod(j_info_class, j_device_info_array_method));
  
  int device_info_count = jni->GetArrayLength(j_device_info_array);
  for (int i = 0; i < device_info_count; i++) {
    jobject j_device_info = jni->GetObjectArrayElement(j_device_info_array, i);
    if (j_device_info == NULL)
      break;
    jfieldID j_name_fieldID = jni->GetFieldID(j_device_info_class, "name", "Ljava/lang/String;");
    jstring j_name_field = static_cast<jstring>(jni->GetObjectField(j_device_info, j_name_fieldID));
    const char* name_field = jni->GetStringUTFChars(j_name_field, 0);
    jfieldID j_frontFacing_fieldID = jni->GetFieldID(j_device_info_class, "frontFacing", "Z");
    jboolean j_frontFacing_field = jni->GetBooleanField(j_device_info, j_frontFacing_fieldID);
    jfieldID j_orientation_fieldID = jni->GetFieldID(j_device_info_class, "orientation", "I");
    jint j_orientation_field = jni->GetIntField(j_device_info, j_orientation_fieldID);
    jfieldID j_sizes_fieldID = jni->GetFieldID(j_device_info_class, "sizes", "[Lorg/webrtc/videoengine/VideoCaptureDeviceInfoAndroid$FrameSize;");
    jobjectArray j_sizes_field = static_cast<jobjectArray>(jni->GetObjectField(j_device_info, j_sizes_fieldID));
    jfieldID j_minMfps_fieldID = jni->GetFieldID(j_device_info_class, "minMfps", "I");
    jint j_minMfps_field = jni->GetIntField(j_device_info, j_minMfps_fieldID);
    jfieldID j_maxMfps_fieldID = jni->GetFieldID(j_device_info_class, "maxMfps", "I");
    jint j_maxMfps_field = jni->GetIntField(j_device_info, j_maxMfps_fieldID);
    AndroidCameraInfo info;
    info.name = name_field;
    info.min_mfps = j_minMfps_field;
    info.max_mfps = j_maxMfps_field;
    info.front_facing = j_frontFacing_field;
    info.orientation = j_orientation_field;
    int size_count = jni->GetArrayLength(j_sizes_field);
    for (int j = 0; j < size_count; j++) {
      jobject j_size = jni->GetObjectArrayElement(j_sizes_field, j);
      if (j_size == NULL)
        break;
      jfieldID j_width_fieldID = jni->GetFieldID(j_size_class, "width", "I");
      jint j_width_field = jni->GetIntField(j_size, j_width_fieldID);
      jfieldID j_height_fieldID = jni->GetFieldID(j_size_class, "height", "I");
      jint j_height_field = jni->GetIntField(j_size, j_height_fieldID);
      info.resolutions.push_back(std::make_pair(j_width_field, j_height_field));
    }
    jni->ReleaseStringUTFChars(j_name_field, name_field);
    g_camera_info->push_back(info);
  }
}

VideoCaptureModule::DeviceInfo* VideoCaptureImpl::CreateDeviceInfo(
    const int32_t id) {
  return new videocapturemodule::DeviceInfoAndroid(id);
}

DeviceInfoAndroid::DeviceInfoAndroid(const int32_t id) :
    DeviceInfoImpl(id) {
}

DeviceInfoAndroid::~DeviceInfoAndroid() {
}

bool DeviceInfoAndroid::FindCameraIndex(const char* deviceUniqueIdUTF8,
                                        size_t* index) {
  return FindCameraIndexByName(deviceUniqueIdUTF8, index);
}

int32_t DeviceInfoAndroid::Init() {
  return 0;
}

uint32_t DeviceInfoAndroid::NumberOfDevices() {
  return g_camera_info->size();
}

int32_t DeviceInfoAndroid::GetDeviceName(
    uint32_t deviceNumber,
    char* deviceNameUTF8,
    uint32_t deviceNameLength,
    char* deviceUniqueIdUTF8,
    uint32_t deviceUniqueIdUTF8Length,
    char* /*productUniqueIdUTF8*/,
    uint32_t /*productUniqueIdUTF8Length*/) {
  if (deviceNumber >= g_camera_info->size())
    return -1;
  const AndroidCameraInfo& info = g_camera_info->at(deviceNumber);
  if (info.name.length() + 1 > deviceNameLength ||
      info.name.length() + 1 > deviceUniqueIdUTF8Length) {
    return -1;
  }
  memcpy(deviceNameUTF8, info.name.c_str(), info.name.length() + 1);
  memcpy(deviceUniqueIdUTF8, info.name.c_str(), info.name.length() + 1);
  return 0;
}

int32_t DeviceInfoAndroid::CreateCapabilityMap(
    const char* deviceUniqueIdUTF8) {
  _captureCapabilities.clear();
  const AndroidCameraInfo* info = FindCameraInfoByName(deviceUniqueIdUTF8);
  if (info == NULL)
    return -1;

  for (size_t i = 0; i < info->resolutions.size(); ++i) {
    const std::pair<int, int>& size = info->resolutions[i];
    VideoCaptureCapability cap;
    cap.width = size.first;
    cap.height = size.second;
    cap.maxFPS = info->max_mfps / 1000;
    cap.expectedCaptureDelay = kExpectedCaptureDelay;
    cap.rawType = kVideoNV21;
    _captureCapabilities.push_back(cap);
  }
  return _captureCapabilities.size();
}

int32_t DeviceInfoAndroid::GetOrientation(
    const char* deviceUniqueIdUTF8,
    VideoCaptureRotation& orientation) {
  const AndroidCameraInfo* info = FindCameraInfoByName(deviceUniqueIdUTF8);
  if (info == NULL ||
      !VideoCaptureImpl::RotationFromDegrees(info->orientation, &orientation)) {
    return -1;
  }
  return 0;
}

void DeviceInfoAndroid::GetFpsRange(const char* deviceUniqueIdUTF8,
                                    int* min_mfps, int* max_mfps) {
  const AndroidCameraInfo* info = FindCameraInfoByName(deviceUniqueIdUTF8);
  if (info == NULL)
    return;
  *min_mfps = info->min_mfps;
  *max_mfps = info->max_mfps;
}

}  // namespace videocapturemodule
}  // namespace webrtc
