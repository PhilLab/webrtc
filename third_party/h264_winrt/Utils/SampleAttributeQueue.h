/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef _H264_SAMPLEATTRIBUTEQUEUE_H_INCLUDED_
#define _H264_SAMPLEATTRIBUTEQUEUE_H_INCLUDED_

#include <list>
#include <utility>
#include <stdint.h>

// A sorted queue with certain properties which makes it
// good for mapping attributes to frames and samples.
// The ids have to be in increasing order.
template <typename T>
class SampleAttributeQueue {
public:
  SampleAttributeQueue() {}
  ~SampleAttributeQueue() {}

  void push(uint64_t id, const T& t) {
    _attributes.push_back(std::make_pair(id, t));
  }

  bool pop(uint64_t id, T& outT) {
    while (!_attributes.empty()) {
      auto entry = _attributes.front();
      if (entry.first > id) {
        outT = entry.second;
        return true;
      }
      else if (entry.first == id) {
        outT = entry.second;
        _attributes.pop_front();
        return true;
      }
      else {
        _attributes.pop_front();
      }
    }
    return false;
  }

private:
  std::list<std::pair<uint64_t, const T>> _attributes;
};

#endif  // _H264_SAMPLEATTRIBUTEQUEUE_H_INCLUDED_
