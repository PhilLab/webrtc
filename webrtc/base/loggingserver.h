#ifndef WEBRTC_BASE_LOGGINGSERVER_H_
#define WEBRTC_BASE_LOGGINGSERVER_H_

#include <list>
#include <utility>
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/sigslot.h"

namespace rtc {
class AsyncSocket;
class SocketAddress;
class SocketStream;

// Inherit from has_slots class to use signal and slots.
class LoggingServer : public sigslot::has_slots<> {
 public:
  LoggingServer();
  virtual ~LoggingServer();

  int Listen(const SocketAddress& addr, int level);

 protected:
  void OnAcceptEvent(AsyncSocket* socket);
  void OnCloseEvent(AsyncSocket* socket, int err);

 private:
  int level_;
  scoped_ptr<AsyncSocket> listener_;
  std::list<std::pair<AsyncSocket*, SocketStream*> > connections_;
};

}  //  namespace rtc

#endif  //  WEBRTC_BASE_LOGGINGSERVER_H_
