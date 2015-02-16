#include "webrtc/base/loggingserver.h"
#include "webrtc/base/socketaddress.h"
#include "webrtc/base/asyncsocket.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/socketstream.h"
#include "webrtc/base/logging.h"

namespace rtc {

LoggingServer::LoggingServer() {
}

LoggingServer::~LoggingServer() {
  if (listener_)
    listener_->Close();

  for (auto it = connections_.begin();
               it != connections_.end(); ++it) {
    it->second->Detach();
    delete it->first;
    delete it->second;
  }
}

int LoggingServer::Listen(const SocketAddress& addr, int level) {
  level_ = level;

  AsyncSocket* sock =
    Thread::Current()->socketserver()->CreateAsyncSocket(AF_INET, SOCK_STREAM);

  if (!sock) {
    return SOCKET_ERROR;
  }

  listener_.reset(sock);
  listener_->SignalReadEvent.connect(this, &LoggingServer::OnAcceptEvent);

  // Bind to the specified address and listen incoming connections.
  // Maximum 5 pending connections are allowed.
  if ((listener_->Bind(addr) != SOCKET_ERROR) &&
    (listener_->Listen(5) != SOCKET_ERROR)) {
    return 0;
  }

  return listener_->GetError();
}

void LoggingServer::OnAcceptEvent(AsyncSocket* socket) {
  if (!listener_)
    return;

  if (socket != listener_.get())
    return;

  AsyncSocket* incoming = listener_->Accept(NULL);

  if (incoming) {
    // Attach the socket of accepted connection to a stream.
    auto stream = new SocketStream(incoming);
    connections_.push_back(std::make_pair(incoming, stream));

    // Add new non-blocking stream to log messages.
    LogMessage::AddLogToStream(stream, level_);
    incoming->SignalCloseEvent.connect(this, &LoggingServer::OnCloseEvent);

    LOG(LS_INFO) << "Successfully connected to the logging server!";
  }
}

void LoggingServer::OnCloseEvent(AsyncSocket* socket, int err) {
  if (!socket)
    return;

  LOG(LS_INFO) << "Connection closed : " << err;

  for (auto it = connections_.begin();
               it != connections_.end(); ++it) {
    if (socket == it->first) {
      LogMessage::RemoveLogToStream(it->second);
      it->second->Detach();

      // Post messages to delete doomed objects
      Thread::Current()->Dispose(it->first);
      Thread::Current()->Dispose(it->second);
      it = connections_.erase(it);
      break;
    }
  }
}

}  // namespace rtc
