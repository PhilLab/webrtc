#ifndef WEBRTC_BASE_LOGGINGSERVER_H_
#define WEBRTC_BASE_LOGGINGSERVER_H_

#include <list>
#include <utility>
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/sigslot.h"
#include "webrtc/base/logging.h"

namespace webrtc {
class ThreadWrapper;
}

namespace rtc {
class AsyncSocket;
class SocketAddress;
class SocketStream;
class Thread;

template <typename Base>
class LogSinkImpl
  : public LogSink,
  public Base {
public:
  LogSinkImpl() {}

  template<typename P>
  explicit LogSinkImpl(P* p) : Base(p) {}

private:
  void OnLogMessage(const std::string& message) override {
    static_cast<Base*>(this)->WriteAll(
      message.data(), message.size(), nullptr, nullptr);
  }
};

// Inherit from has_slots class to use signal and slots.
class LoggingServer : public sigslot::has_slots<sigslot::multi_threaded_local> {
 public:
  LoggingServer();
  virtual ~LoggingServer();

  int Listen(const SocketAddress& addr, LoggingSeverity level);

 protected:
  void OnAcceptEvent(AsyncSocket* socket);
  void OnCloseEvent(AsyncSocket* socket, int err);

 private:
  static bool processMessages(void* args);

 private:
   LoggingSeverity level_;
  scoped_ptr<AsyncSocket> listener_;
  std::list<std::pair<AsyncSocket*, LogSinkImpl<SocketStream>* > > connections_;
  Thread* thread_;
  scoped_ptr<webrtc::ThreadWrapper> tw_;
};

}  //  namespace rtc

#endif  //  WEBRTC_BASE_LOGGINGSERVER_H_
