#ifndef WEBRTC_BASE_TRACELOG_H_
#define WEBRTC_BASE_TRACELOG_H_

#include <string>
#include <sstream>
#include <vector>

#include "webrtc/base/scoped_ptr.h"
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
class TraceLog : public sigslot::has_slots<sigslot::multi_threaded_local> {
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
  void StopTracing();
  bool IsTracing();

  bool Save(const std::string& file_name);
  bool Save(const std::string& addr, int port);

 private:
  static bool processMessages(void* args);

 private:
  void OnCloseEvent(AsyncSocket* socket, int err);
  void OnWriteEvent(AsyncSocket* socket);

  void SaveTraceChunk();
  void CleanTracesStorage();
  bool LoadFirstTraceChunk();
  bool LoadNextTraceChunk();

 private:
  bool is_tracing_;
  unsigned int offset_;

  // Enable to store traces locally if exceeds configured memory limit
  bool traces_storage_enabled_;
  unsigned int traces_memory_limit_;
  std::string traces_storage_file_;

  unsigned int send_chunk_size_;
  unsigned int send_chunk_offset_;
  unsigned int send_max_chunk_size_;
  unsigned int stored_traces_size_;
  unsigned int sent_bytes_;
  std::vector<char> send_chunk_buffer_;

  std::ostringstream oss_;
  CriticalSection critical_section_;
  Thread* thread_;
  scoped_ptr<webrtc::ThreadWrapper> tw_;
};

}  // namespace rtc


#endif  //  WEBRTC_BASE_TRACELOG_H_
