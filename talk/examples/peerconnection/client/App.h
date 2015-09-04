#pragma once

#include "WinRTMainWnd.h"
#include "conductor.h"
#include "webrtc/base/ssladapter.h"
#include <collection.h>
#include <ppltasks.h>
#include <string>

using namespace Windows::UI;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Core;
using namespace Windows::Foundation::Collections;

class WinRTMainWnd;

namespace peerconnectionclient
{
	public ref class App sealed : public Application
	{
	public:
		App();
		virtual ~App();		

	internal:
		//WinRTMainWnd callbacks
		void MessageBox(::Platform::String^ caption, ::Platform::String^ text, bool is_error);
		void UpdatePeersList(IMap<int32, Platform::String^>^ peers);
		void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
		void StopLocalRenderer();
		void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
		void StopRemoteRenderer();

	protected:
		void OnLaunched(LaunchActivatedEventArgs^ args) override;

	private:
		InputScope^ CreateInputScope();
		void OnStartStopClick(Platform::Object ^sender, RoutedEventArgs ^e);

		TextBlock^ ipLabel_;
		TextBox^ ipTextBox_;
		TextBlock^ portLabel_;
		TextBox^ portTextBox_;
		Button^ connectButton_;
		TextBlock^ peersLabel_;
		ListView^ peersListBox_;
		MediaElement^ localMedia_;
		MediaElement^ remoteMedia_;
		CoreDispatcher^ dispatcher_;
		WinRTMainWnd* mainWnd_;
		PeerConnectionClient* peerConnectionClient_;
		rtc::RefCountedObject<Conductor>* conductor_;
    rtc::Thread workerThread_;
	};
}