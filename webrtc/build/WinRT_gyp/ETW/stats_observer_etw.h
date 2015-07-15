
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBRTC_BUILD_WINRT_GYP_ETW_STATS_OBSERVER_ETW_H_
#define WEBRTC_BUILD_WINRT_GYP_ETW_STATS_OBSERVER_ETW_H_

#include "talk/app/webrtc/peerconnectioninterface.h"

namespace webrtc {
// A webrtc::StatsObserver implementation used to receive statistics about the
// current PeerConnection. The statistics are logged to an ETW session.
class StatsObserverETW : public StatsObserver, public rtc::MessageHandler {
 public:
  StatsObserverETW();
  void PollStats(rtc::scoped_refptr<webrtc::PeerConnectionInterface> pci);

  // StatsObserver
  virtual void OnComplete(const StatsReports& reports);

  // MessageHandler
  void OnMessage(rtc::Message* msg);

 protected:
  virtual ~StatsObserverETW();

 private:
  void GetStreamCollectionStats(rtc::scoped_refptr<StreamCollectionInterface> streams);

 private:
  static const int kInterval;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pci_;
};

}  // namespace webrtc

#endif  //  WEBRTC_BUILD_WINRT_GYP_ETW_STATS_OBSERVER_ETW_H_
