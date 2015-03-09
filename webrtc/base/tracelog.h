#ifndef WEBRTC_BASE_TRACELOG_H_
#define WEBRTC_BASE_TRACELOG_H_

#include <vector>
#include <string>

#include "webrtc/base/sigslot.h"
#include "webrtc/base/criticalsection.h"

namespace webrtc {
class ThreadWrapper;
}  // namespace webrtc

namespace rtc {
class AsyncSocket;
class Thread;

// Aggregates traces. Allows to save traces in local file
// and send them to remote host. To start aggregating traces
// call StartTracing method. Before saving the data locally or
// remotely make sure you have called StopTracing method.
class TraceLog : public sigslot::has_slots<> {
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
  static bool processMessages(void* args);

 private:
  void OnCloseEvent(AsyncSocket* socket, int err);
  void OnWriteEvent(AsyncSocket* socket);

 private:
  bool is_tracing_;
  std::vector<std::string> traces_;
  CriticalSection critical_section_;
  Thread* thread_;
  webrtc::ThreadWrapper* tw_;
};

}  // namespace rtc


#endif  //  WEBRTC_BASE_TRACELOG_H_
