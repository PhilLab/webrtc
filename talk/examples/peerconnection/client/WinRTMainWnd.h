#pragma once

#include "main_wnd.h"
#include "App.h"

namespace peerconnectionclient { ref class App; }

class WinRTMainWnd : public MainWindow
{
public:

	// MainWindow redeclarations
	virtual void RegisterObserver(MainWndCallback* callback);

	virtual bool IsWindow();
	virtual void MessageBox(const char* caption, const char* text, bool is_error);

	virtual UI current_ui();

	virtual void SwitchToConnectUI();
	virtual void SwitchToPeerList(const Peers& peers);
	virtual void SwitchToStreamingUI();

	virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
	virtual void StopLocalRenderer();
	virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
	virtual void StopRemoteRenderer();

	virtual void QueueUIThreadCallback(int msg_id, void* data);

	//winrt parent app
	void RegisterParentApp(peerconnectionclient::App^ app);
	void StartLogin(Platform::String ^server, int32 port);
	void DisconnectFromServer();
	void ConnectToPeer(int peer_id);
	void DisconnectFromCurrentPeer();
	//void UIThreadCallback(int msg_id, void* data);
	void Close();

private:
	std::string make_string(const std::wstring& wstring);
	std::wstring make_wstring(const std::string& string);

	MainWndCallback* callback_;
	peerconnectionclient::App^ parentApp_;
};