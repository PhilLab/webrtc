/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package org.webrtc.videoengine;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import android.content.Context;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.hardware.Camera;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class VideoCaptureDeviceInfoAndroid {
  private final static String TAG = "WEBRTC-JC";
  
  private static class FrameSize {
	  int width;
	  int height;
  }
  
  private static class DeviceInfo {
	  String name;
	  boolean frontFacing;
	  int orientation;
	  FrameSize[] sizes = new FrameSize[10];
	  int minMfps;
	  int maxMfps;
  }

  private static boolean isFrontFacing(CameraInfo info) {
    return info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT;
  }

  private static String deviceUniqueName(int index, CameraInfo info) {
    return "Camera " + index +", Facing " +
        (isFrontFacing(info) ? "front" : "back") +
        ", Orientation "+ info.orientation;
  }

  // Returns information about all cameras on the device as a serialized JSON
  // array of dictionaries encoding information about a single device.  Since
  // this reflects static information about the hardware present, there is no
  // need to call this function more than once in a single process.  It is
  // marked "private" as it is only called by native code.
  private static String getDeviceInfo() {
    try {
      JSONArray devices = new JSONArray();
      for (int i = 0; i < Camera.getNumberOfCameras(); ++i) {
        CameraInfo info = new CameraInfo();
        Camera.getCameraInfo(i, info);
        String uniqueName = deviceUniqueName(i, info);
        JSONObject cameraDict = new JSONObject();
        devices.put(cameraDict);
        List<Size> supportedSizes;
        List<int[]> supportedFpsRanges;
        try {
          Camera camera = Camera.open(i);
          Parameters parameters = camera.getParameters();
          supportedSizes = parameters.getSupportedPreviewSizes();
          supportedFpsRanges = parameters.getSupportedPreviewFpsRange();
          camera.release();
          Log.d(TAG, uniqueName);
        } catch (RuntimeException e) {
          Log.e(TAG, "Failed to open " + uniqueName + ", skipping");
          continue;
        }
        JSONArray sizes = new JSONArray();
        for (Size supportedSize : supportedSizes) {
          JSONObject size = new JSONObject();
          size.put("width", supportedSize.width);
          size.put("height", supportedSize.height);
          sizes.put(size);
        }
        // Android SDK deals in integral "milliframes per second"
        // (i.e. fps*1000, instead of floating-point frames-per-second) so we
        // preserve that through the Java->C++->Java round-trip.
        int[] mfps = supportedFpsRanges.get(supportedFpsRanges.size() - 1);
        cameraDict.put("name", uniqueName);
        cameraDict.put("front_facing", isFrontFacing(info))
            .put("orientation", info.orientation)
            .put("sizes", sizes)
            .put("min_mfps", mfps[Parameters.PREVIEW_FPS_MIN_INDEX])
            .put("max_mfps", mfps[Parameters.PREVIEW_FPS_MAX_INDEX]);
      }
      String ret = devices.toString(2);
      return ret;
    } catch (JSONException e) {
      throw new RuntimeException(e);
    }
  }
  
  private static DeviceInfo[] getDeviceInfoArray() {
    DeviceInfo[] deviceInfoArray = new DeviceInfo[4];
    for (int i = 0; i < Camera.getNumberOfCameras() && i < 4; ++i) {
      DeviceInfo deviceInfo = new DeviceInfo();
      CameraInfo info = new CameraInfo();
      Camera.getCameraInfo(i, info);
      String uniqueName = deviceUniqueName(i, info);
      List<Size> supportedSizes;
      List<int[]> supportedFpsRanges;
      try {
        Camera camera = Camera.open(i);
        Parameters parameters = camera.getParameters();
        supportedSizes = parameters.getSupportedPreviewSizes();
        supportedFpsRanges = parameters.getSupportedPreviewFpsRange();
        camera.release();
        Log.d(TAG, uniqueName);
      } catch (RuntimeException e) {
        Log.e(TAG, "Failed to open " + uniqueName + ", skipping");
        continue;
      }
      int[] mfps = supportedFpsRanges.get(supportedFpsRanges.size() - 1);
      deviceInfo.name = uniqueName;
      deviceInfo.frontFacing = isFrontFacing(info);
      deviceInfo.orientation = info.orientation;
      deviceInfo.minMfps = mfps[Parameters.PREVIEW_FPS_MIN_INDEX];
      deviceInfo.maxMfps = mfps[Parameters.PREVIEW_FPS_MAX_INDEX];
      for (int j = 0; j < supportedSizes.size() && j < 10; ++j) {
        Size supportedSize = supportedSizes.get(j);
        FrameSize frameSize = new FrameSize();
        frameSize.width = supportedSize.width;
        frameSize.height = supportedSize.height;
        deviceInfo.sizes[j] = frameSize;
      }
      deviceInfoArray[i] = deviceInfo;
    }
    return deviceInfoArray;
  }
}
