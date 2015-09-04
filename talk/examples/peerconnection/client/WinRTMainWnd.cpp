#include "main_wnd.h"
#include "WinRTMainWnd.h"
#include <collection.h>

using namespace Platform::Collections;

void WinRTMainWnd::RegisterObserver(MainWndCallback* callback)
{
	callback_ = callback;
}

void WinRTMainWnd::StartLogin(Platform::String ^server, int32 port)
{
	auto s_server = make_string(server->Data());
	callback_->StartLogin(s_server, port);
}

void WinRTMainWnd::DisconnectFromServer()
{
	callback_->DisconnectFromServer();
}

void WinRTMainWnd::ConnectToPeer(int peer_id)
{
	callback_->ConnectToPeer(peer_id);
}

void WinRTMainWnd::DisconnectFromCurrentPeer()
{
	callback_->DisconnectFromCurrentPeer();
}

bool WinRTMainWnd::IsWindow()
{
	return true;
}

void WinRTMainWnd::MessageBox(const char* caption, const char* text, bool is_error)
{
	auto w_caption = make_wstring(caption);
	auto w_text = make_wstring(text);

	auto platform_caption = ref new Platform::String(w_caption.data());
	auto platform_text = ref new Platform::String(w_text.data());

	parentApp_->MessageBox(platform_caption, platform_text, is_error);
}

MainWindow::UI WinRTMainWnd::current_ui()
{
	return UI::MONOLITH_UI;
}

void WinRTMainWnd::SwitchToConnectUI() { }

void WinRTMainWnd::SwitchToPeerList(const Peers& peers)
{
	Map<int32, Platform::String^>^ map = ref new Map<int32, Platform::String^>();

	for (auto pair : peers)
	{
		auto w_peer = make_wstring(pair.second);
		auto platform_peer = ref new Platform::String(w_peer.data());
		map->Insert(pair.first, platform_peer);
	}

	parentApp_->UpdatePeersList(map);
}

void WinRTMainWnd::SwitchToStreamingUI() { }

void WinRTMainWnd::StartLocalRenderer(webrtc::VideoTrackInterface* local_video)
{
	parentApp_->StartLocalRenderer(local_video);
}

void WinRTMainWnd::StopLocalRenderer()
{
	parentApp_->StopLocalRenderer();
}

void WinRTMainWnd::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video)
{
	parentApp_->StartRemoteRenderer(remote_video);
}

void WinRTMainWnd::StopRemoteRenderer()
{
	parentApp_->StopRemoteRenderer();
}

extern CoreDispatcher^ g_windowDispatcher;

void WinRTMainWnd::QueueUIThreadCallback(int msg_id, void* data)
{
	//parentApp_->QueueUIThreadCallback(msg_id, data);
  g_windowDispatcher->RunAsync(CoreDispatcherPriority::Normal,
    ref new DispatchedHandler([this, msg_id, data] {
    callback_->UIThreadCallback(msg_id, data);
  }));

}

void WinRTMainWnd::RegisterParentApp(peerconnectionclient::App^ app)
{
	parentApp_ = app;
}

std::string WinRTMainWnd::make_string(const std::wstring& wstring)
{
	auto wideData = wstring.c_str();
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideData, -1, nullptr, 0, NULL, NULL);
	auto utf8 = std::make_unique<char[]>(bufferSize);
	if (0 == WideCharToMultiByte(CP_UTF8, 0, wideData, -1, utf8.get(), bufferSize, NULL, NULL))
		throw std::exception("Can't convert string to UTF8");

	return std::string(utf8.get());
}

std::wstring WinRTMainWnd::make_wstring(const std::string& string)
{
	auto utf8Data = string.c_str();
	int bufferSize = MultiByteToWideChar(CP_UTF8, 0, utf8Data, -1, nullptr, 0);
	auto wide = std::make_unique<wchar_t[]>(bufferSize);
	if (0 == MultiByteToWideChar(CP_UTF8, 0, utf8Data, -1, wide.get(), bufferSize))
		throw std::exception("Can't convert string to Unicode");

	return std::wstring(wide.get());
}
