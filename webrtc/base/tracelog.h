#ifndef WEBRTC_BASE_TRACELOG_H_
#define WEBRTC_BASE_TRACELOG_H_

#include <vector>

#include "webrtc/base/criticalsection.h"

namespace rtc {

class TraceLog {
public:
  TraceLog();
  virtual ~TraceLog();

  void Add(char phase,
    const unsigned char* category_group_enabled,
    const char* name,
    unsigned long long id,
    int num_args,
    const char** arg_names,
    const unsigned char* arg_types,
    const unsigned long long* arg_values,
    unsigned char flags);

  void StartTracing();
  void StopStracing();
  bool IsTracing();

  void Save(const std::string& file_name);
  void Save(const std::string& addr, int port);

private:
  bool is_tracing_;
  std::vector<std::string> traces_;
  CriticalSection critical_section_;
};

}  // namespace rtc


#endif  //  WEBRTC_BASE_TRACELOG_H_