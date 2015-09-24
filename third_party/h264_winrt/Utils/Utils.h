#ifndef UTILS_H_
#define UTILS_H_

#include <Windows.h>

inline void ThrowIfError(HRESULT hr) {
  if (FAILED(hr)) {
    throw ref new Platform::Exception(hr);
  }
}

inline void Throw(HRESULT hr) {
  throw ref new Platform::Exception(hr);
}

#endif  // UTILS_H_