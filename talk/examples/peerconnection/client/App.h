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

		//WinRTMainWnd callbacks
		void MessageBox(::Platform::String^ caption, ::Platform::String^ text, bool is_error);
		void UpdatePeersList(IMap<int32, Platform::String^>^ peers);

	protected:
		void OnLaunched(LaunchActivatedEventArgs^ args) override;

	private:
		InputScope^ CreateInputScope();

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
		rtc::scoped_refptr<Conductor> conductor_;
	};
}