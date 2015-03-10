#include "webrtc/base/loggingserver.h"
#include "webrtc/base/socketaddress.h"
#include "webrtc/base/asyncsocket.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/socketstream.h"
#include "webrtc/base/logging.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"

#if defined(WEBRTC_WIN) && !defined(WINRT)
#include "webrtc/base/win32socketserver.h"
#endif
#if defined(WEBRTC_POSIX)
#include "webrtc/base/physicalsocketserver.h"
#endif

namespace rtc {

LoggingServer::LoggingServer() {
#if defined(WEBRTC_WIN) && !defined(WINRT)
  thread_ = new Win32Thread();
#endif
#if defined(WEBRTC_POSIX)
  PhysicalSocketserver* pss = new PhysicalSocketServer();
  thread_ = new Thread(pss);
#endif
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

  tw_->Stop();
  thread_->Stop();

  delete tw_;
}

int LoggingServer::Listen(const SocketAddress& addr, int level) {
  level_ = level;
  tw_ = webrtc::ThreadWrapper::CreateThread(&LoggingServer::processMessages, thread_);
  unsigned int id;
  tw_->Start(id);

  LOG(LS_INFO) << "New thread created with id: " << id;

  AsyncSocket* sock =
    thread_->socketserver()->CreateAsyncSocket(AF_INET, SOCK_STREAM);

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

bool LoggingServer::processMessages(void* args) {
  Thread* t = reinterpret_cast<Thread*>(args);
  t->Run();
  return true;
}

}  // namespace rtc
