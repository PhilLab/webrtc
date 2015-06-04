#include <sstream>
#include <fstream>

#include "webrtc/base/tracelog.h"
#include "webrtc/base/asyncsocket.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/timeutils.h"
#include "webrtc/system_wrappers/interface/trace_event.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"
#include "webrtc/base/physicalsocketserver.h"

namespace rtc {

TraceLog::TraceLog() : is_tracing_(false), offset_(0), tw_() {
  PhysicalSocketServer* pss = new PhysicalSocketServer();
  thread_ = new Thread(pss);
}

TraceLog::~TraceLog() {
  if (tw_) {
    tw_->Stop();
    thread_->Stop();
  }
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
    << "\"ts\": " << rtc::TimeMicros() << ", "
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

  t << "}" << "},";

  CritScope lock(&critical_section_);
  oss_ << t.str();
}

void TraceLog::StartTracing() {
  if (!is_tracing_) {
    CritScope lock(&critical_section_);
    oss_.clear();
    oss_.str("");
    oss_ << "{ \"traceEvents\": [";
    is_tracing_ = true;
  }
}

void TraceLog::StopTracing() {
  if (is_tracing_) {
    CritScope lock(&critical_section_);
    long pos = oss_.tellp();
    oss_.seekp(pos - 1);
    oss_ << "]}";
    is_tracing_ = false;
  }
}

bool TraceLog::IsTracing() {
  return is_tracing_;
}

bool TraceLog::Save(const std::string& file_name) {
  bool result = true;
  std::ofstream file;
  file.open(file_name.c_str());
  if (file.is_open())
  {
    file << oss_.str();
    file.close();
    if (file.bad())
      result = false;
  }
  else
    result = false;
  return result;
}

bool TraceLog::Save(const std::string& addr, int port) {
  if (offset_) {
    // Sending the data still is in progress.
    return false;
  }

  if (tw_ == NULL) {
    tw_ = webrtc::ThreadWrapper::CreateThread(&TraceLog::processMessages, thread_, "TraceLog");
    tw_->Start();

    LOG(LS_INFO) << "New TraceLog thread created.";
  }

  AsyncSocket* sock =
    thread_->socketserver()->CreateAsyncSocket(AF_INET, SOCK_STREAM);
  sock->SignalWriteEvent.connect(this, &TraceLog::OnWriteEvent);
  sock->SignalCloseEvent.connect(this, &TraceLog::OnCloseEvent);

  SocketAddress server_addr(addr, port);
  sock->Connect(server_addr);

  // Send wake up signal to update the event list to wait
  thread_->socketserver()->WakeUp();
  return true;
}

void TraceLog::OnCloseEvent(AsyncSocket* socket, int err) {
  if (!socket)
    return;

  SocketAddress addr = socket->GetRemoteAddress();
  LOG(LS_ERROR) << "The connection was closed. "
    << "IP: " << addr.HostAsURIString() << ", "
    << "Port: " << addr.port() << ", "
    << "Error: " << err;

  offset_ = 0;
  thread_->Dispose(socket);
}

void TraceLog::OnWriteEvent(AsyncSocket* socket) {
  if (!socket)
    return;

  const std::string& tmp = oss_.str();
  size_t tmp_size = tmp.size();
  const char* data = tmp.c_str();

  int sent_size = 0;
  while (offset_ < tmp_size) {
    sent_size = socket->Send((const void*) (data + offset_), tmp_size - offset_);
    if (sent_size == -1) {
      if (!IsBlockingError(socket->GetError())) {
        offset_ = 0;
        socket->Close();
      }
      break;
    } else {
      offset_ += sent_size;
    }
  }

  if (tmp_size == offset_) {
    offset_ = 0;
    socket->Close();
  }
}

bool TraceLog::processMessages(void* args) {
  Thread* t = reinterpret_cast<Thread*>(args);
  if (t)
    t->Run();
  return true;
}

}  //  namespace rtc
