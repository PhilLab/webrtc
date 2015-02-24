#include <sstream>
#include <fstream>

#include "webrtc/base/tracelog.h"
#include "webrtc/base/timeutils.h"
#include "webrtc/system_wrappers/interface/trace_event.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"

namespace rtc {

TraceLog::TraceLog() : is_tracing_(false) {
  traces_.reserve(16384);
}

TraceLog::~TraceLog() {
}


void TraceLog::Add(char phase,
  const unsigned char* category_group_enabled,
  const char* name,
  unsigned long long id,
  int num_args,
  const char** arg_names,
  const unsigned char* arg_types,
  const unsigned long long* arg_values,
  unsigned char flags) {
  if (!is_tracing_)
    return;

  std::ostringstream t;
  t << "{"
    << "\"name\": \"" << name << "\", "
    << "\"cat\": \"" << category_group_enabled << "\", "
    << "\"ph\": \"" << phase << "\", "
    << "\"ts\": " << rtc::Time() << ", "
    << "\"pid\": " << 0 << ", "
    << "\"tid\": " << webrtc::ThreadWrapper::GetThreadId() << ", "
    << "\"args\": {";

  webrtc::trace_event_internal::TraceValueUnion tvu;

  for (int i = 0; i < num_args; ++i) {
    t << "\"" << arg_names[i] << "\": ";
    tvu.as_uint = arg_values[i];

    switch (arg_types[i]) {
    case TRACE_VALUE_TYPE_BOOL:
      t << tvu.as_bool;
      break;
    case TRACE_VALUE_TYPE_UINT:
      t << tvu.as_uint;
      break;
    case TRACE_VALUE_TYPE_INT:
      t << tvu.as_int;
      break;
    case TRACE_VALUE_TYPE_DOUBLE:
      t << tvu.as_double;
      break;
    case TRACE_VALUE_TYPE_POINTER:
      t << tvu.as_pointer;
      break;
    case TRACE_VALUE_TYPE_STRING:
    case TRACE_VALUE_TYPE_COPY_STRING:
      t << "\"" << tvu.as_string << "\"";
      break;
    default:
      break;
    }

    if (i < num_args - 1) {
      t << ", ";
    }
  }

  t << "}" << "}";

  CritScope lock(&critical_section_);
  traces_.push_back(t.str());
}

void TraceLog::StartTracing() {
  CritScope lock(&critical_section_);
  traces_.clear();
  is_tracing_ = true;
}

void TraceLog::StopStracing() {
  is_tracing_ = false;
}

bool TraceLog::IsTracing() {
  return is_tracing_;
}

void TraceLog::Save(const std::string& file_name) {
  std::ofstream file;
  file.open(file_name);
  file << "{ \"traceEvents\": [";

  int traces_size = traces_.size();
  for (int i = 0; i < traces_size; ++i) {
    file << traces_[i];
    if (i < traces_size - 1) {
      file << ", ";
    }
  }

  file << "]}";
  file.close();
}

void TraceLog::Save(const std::string& addr, int port) {
}

}  //  namespace rtc
