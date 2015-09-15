<?xml version="1.0"?><doc>
<members>
<member name="M:webrtc.IceCandidateInterface.sdp_mid" decl="false" source="c:\work\webrtc\repositories\webrtc\src\talk\app\webrtc\jsep.h" line="60">
If present, this contains the identierfier of the "media stream
</member>
<member name="T:webrtc_winrt_api.RTCDataChannelState" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="25">
<summary>
Used to keep track of a <see cref="T:webrtc_winrt_api.RTCDataChannel"/>'s state.
</summary>
</member>
<member name="T:webrtc_winrt_api.RTCDataChannelInit" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="35">
<summary>
Can be used to configure properties of the underlying channel such as data reliability.
</summary>
</member>
<member name="T:webrtc_winrt_api.RTCDataChannelMessageType" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="48">
<summary>
Type of messages for a <see cref="T:webrtc_winrt_api.RTCDataChannel"/>
</summary>
</member>
<member name="T:webrtc_winrt_api.StringDataChannelMessage" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="60">
<summary>
Message type used for sending strings (chat messages, for example) over a data channel.
</summary>
</member>
<member name="T:webrtc_winrt_api.BinaryDataChannelMessage" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="75">
<summary>
Message type used for sending binary data (a file, for example) over a data channel.
</summary>
</member>
<member name="T:webrtc_winrt_api.RTCDataChannel" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="95">
<summary>
Represents a bi-directional data channel between two peers.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.Label" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="105">
<summary>
Can be used to distinguish this <see cref="T:webrtc_winrt_api.RTCDataChannel"/> object from other <see cref="T:webrtc_winrt_api.RTCDataChannel"/> 
objects. Uniqueness is not guaranteed for labels.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.Ordered" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="111">
<summary>
true if the<see cref="T:webrtc_winrt_api.RTCDataChannel"/> is ordered, and false if other of order delivery is allowed.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.MaxPacketLifeTime" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="116">
<summary>
Length of the time window (in milliseconds) during which transmissions and retransmissions may occur in 
unreliable mode, or nullptr if unset. 
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.MaxRetransmits" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="122">
<summary>
Maximum number of retransmissions that are attempted in unreliable mode, or nullptr if unset.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.Protocol" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="127">
<summary>
The name of the sub-protocol used with this RTCDataChannel if any.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.Negotiated" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="132">
<summary>
true if this RTCDataChannel was negotiated by the application, false otherwise.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.Id" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="137">
<summary>
Unique identifier for the data channel.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.ReadyState" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="142">
<summary>
The state of the RTCDataChannel object.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCDataChannel.BufferedAmount" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="147">
<summary>
The number of bytes of application data (UTF-8 text and binary data) that have been queued but that, 
as of the last time the event loop started executing a task, had not yet been transmitted to the network.
</summary>
</member>
<member name="E:webrtc_winrt_api.RTCDataChannel.OnMessage" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="153">
<summary>
Event triggered when a message is successfully received.
</summary>
</member>
<member name="E:webrtc_winrt_api.RTCDataChannel.OnOpen" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="158">
<summary>
Event triggered when a data channel is opened.
</summary>
</member>
<member name="E:webrtc_winrt_api.RTCDataChannel.OnClose" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="163">
<summary>
Event triggered when a data channel is closed.
</summary>
</member>
<member name="M:webrtc_winrt_api.RTCDataChannel.Send(webrtc_winrt_api.IDataChannelMessage)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\datachannel.h" line="173">
<summary>
Attempts to send data on channel's underlying data transport.
</summary>
<param name="data"></param>
</member>
<member name="T:webrtc_winrt_api.RTCSdpType" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="170">
<summary>
Describes the type of an SDP blob.
</summary>
</member>
<member name="T:webrtc_winrt_api.RTCSessionDescription" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="213">
<summary>
An SDP blob and an associated <see cref="T:webrtc_winrt_api.RTCSdpType"/>.
</summary>
</member>
<member name="T:webrtc_winrt_api.RTCPeerConnection" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="237">
<summary>
An RTCPeerConnection allows two users to communicate directly.
Communications are coordinated via a signaling channel which is provided
by unspecified means.
</summary>
<remarks>
http://www.w3.org/TR/webrtc/#peer-to-peer-connections
</remarks>
</member>
<member name="M:webrtc_winrt_api.RTCPeerConnection.#ctor(webrtc_winrt_api.RTCConfiguration)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="251">
<summary>
Creates an RTCPeerConnection object.
</summary>
<remarks>
Refer to http://www.w3.org/TR/webrtc for the RTCPeerConnection
construction algorithm
</remarks>
<param name="configuration">
The configuration has the information to find and access the
servers used by ICE.
</param>
</member>
<member name="M:webrtc_winrt_api.RTCPeerConnection.CreateOffer" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="271">
<summary>
Generates a blob of SDP that contains an RFC 3264 offer with the
supported configurations for the session, including descriptions
of the local MediaStreams attached to this
<see cref="T:webrtc_winrt_api.RTCPeerConnection"/>,
the codec/RTP/RTCP options supported by this implementation, and
any candidates that have been gathered by the ICE Agent.
</summary>
<returns></returns>
</member>
<member name="M:webrtc_winrt_api.RTCPeerConnection.CreateAnswer" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="282">
<summary>
Generates an SDP answer with the supported configuration for the
session that is compatible with the parameters in the remote
configuration. Like createOffer, the returned blob contains descriptions
of the local MediaStreams attached to this
<see cref="T:webrtc_winrt_api.RTCPeerConnection"/>, the codec/RTP/RTCP options negotiated
for this session, and any candidates that have been gathered by the ICE
Agent.
</summary>
<returns>An action which completes asynchronously</returns>
</member>
<member name="M:webrtc_winrt_api.RTCPeerConnection.SetLocalDescription(webrtc_winrt_api.RTCSessionDescription)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="294">
<summary>
Instructs the <see cref="T:webrtc_winrt_api.RTCPeerConnection"/> to apply the supplied
<see cref="T:webrtc_winrt_api.RTCSessionDescription"/> as the local description.
This API changes the local media state.
</summary>
<param name="description">RTCSessionDescription to apply as the
local description</param>
<returns>An action which completes asynchronously</returns>
</member>
<member name="M:webrtc_winrt_api.RTCPeerConnection.SetRemoteDescription(webrtc_winrt_api.RTCSessionDescription)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="304">
<summary>
Instructs the <see cref="T:webrtc_winrt_api.RTCPeerConnection"/> to apply the supplied
<see cref="T:webrtc_winrt_api.RTCSessionDescription"/> as the remote offer or answer.
This API changes the local media state.
</summary>
<param name="description"><see cref="T:webrtc_winrt_api.RTCSessionDescription"/> to
apply as the local description</param>
<returns>An action which completes asynchronously</returns>
</member>
<member name="M:webrtc_winrt_api.RTCPeerConnection.CreateDataChannel(System.String,webrtc_winrt_api.RTCDataChannelInit)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="321">
<summary>
Creates a new <see cref="T:webrtc_winrt_api.RTCDataChannel"/> object with the given <paramref name="label"/>.
</summary>
<param name="label">Used as the descriptive name for the new data channel</param>
<param name="init">Can be used to configure properties of the underlying channel such as data reliability.</param>
<returns>The newly created <see cref="T:webrtc_winrt_api.RTCDataChannel"/>.</returns>
</member>
<member name="M:webrtc_winrt_api.RTCPeerConnection.AddIceCandidate(webrtc_winrt_api.RTCIceCandidate)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="329">
<summary>
Provides a remote candidate to the ICE Agent.
The candidate is added to the remote description.
This call will result in a change to the connection state of the ICE
Agent, and may lead to a change to media state if it results in
different connectivity being established.
</summary>
<param name="candidate">candidate to be added to the remote description
</param>
<returns>An action which completes asynchronously</returns>
</member>
<member name="P:webrtc_winrt_api.RTCPeerConnection.LocalDescription" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="342">
<summary>
The last <see cref="T:webrtc_winrt_api.RTCSessionDescription"/> that was successfully set
using <see cref="M:webrtc_winrt_api.RTCPeerConnection.SetLocalDescription(webrtc_winrt_api.RTCSessionDescription)"/>, plus any local candidates that
have been generated by the ICE Agent since then.
A nullptr handle will be returned if the local description has not yet
been set.
</summary>
</member>
<member name="P:webrtc_winrt_api.RTCPeerConnection.RemoteDescription" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.h" line="353">
<summary>
The last <see cref="T:webrtc_winrt_api.RTCSessionDescription"/> that was successfully set
using <see cref="M:webrtc_winrt_api.RTCPeerConnection.SetRemoteDescription(webrtc_winrt_api.RTCSessionDescription)"/>,
plus any remote candidates that have been supplied via
<see cref="M:webrtc_winrt_api.RTCPeerConnection.AddIceCandidate(webrtc_winrt_api.RTCIceCandidate)"/> since then.
A nullptr handle will be returned if the local description has not
yet been set.
</summary>
</member>
<member name="T:webrtc_winrt_api.IMediaStreamTrack" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="32">
<summary>
An IMediaStreamTrack object represents media of a single type that originates 
from one media source, e.g. video produced by a web camera.
</summary>
<remarks>
http://www.w3.org/TR/mediacapture-streams
</remarks>
</member>
<member name="P:webrtc_winrt_api.IMediaStreamTrack.Kind" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="41">
<summary>
Type of media, "audio" or "video"
</summary>
</member>
<member name="P:webrtc_winrt_api.IMediaStreamTrack.Id" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="46">
<summary>
Identifier
</summary>
</member>
<member name="P:webrtc_winrt_api.IMediaStreamTrack.Enabled" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="51">
<summary>
Controls the enabled state for the track. 
</summary>
</member>
<member name="M:webrtc_winrt_api.IMediaStreamTrack.Stop" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="56">
<summary>
TODO(WINRT): check implementations, they may not be conforming to the spec: 
http://www.w3.org/TR/mediacapture-streams/#widl-MediaStreamTrack-stop-void
</summary>
</member>
<member name="T:webrtc_winrt_api.MediaVideoTrack" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="63">
<summary>
Represents video media that originates from one video source
</summary>
<seealso cref="T:webrtc_winrt_api.IMediaStreamTrack"/>
</member>
<member name="T:webrtc_winrt_api.MediaAudioTrack" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="86">
<summary>
Represents audio media that originates from one audio source
</summary>
<seealso cref="T:webrtc_winrt_api.IMediaStreamTrack"/>
</member>
<member name="T:webrtc_winrt_api.MediaStream" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="105">
<summary>
A MediaStream is used to group several <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects 
into one unit that can be recorded or rendered in a media element.
Each MediaStream can contain zero or more <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects.
</summary>
<remarks>
http://www.w3.org/TR/mediacapture-streams/
</remarks>
</member>
<member name="M:webrtc_winrt_api.MediaStream.#ctor(rtc.scoped_refptr&lt;webrtc.MediaStreamInterface&gt;)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="116">
<summary>
Composes a new stream
</summary>
<param name="impl"></param>
</member>
<member name="M:webrtc_winrt_api.MediaStream.GetAudioTracks" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="125">
<summary>
Returns a Vector that represents a snapshot of all the <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects
in this stream's track set whose kind is equal to "audio".
</summary>
<returns>A Vector of <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects representing the audio tracks in this stream</returns>
</member>
<member name="M:webrtc_winrt_api.MediaStream.GetVideoTracks" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="132">
<summary>
Returns a Vector that represents a snapshot of all the <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects
in this stream's track set whose kind is equal to "video".
</summary>
<returns>A Vector of <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects representing the video tracks in this stream</returns>
</member>
<member name="M:webrtc_winrt_api.MediaStream.GetTracks" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="139">
<summary>
Returns a Vector that represents a snapshot of all the <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects 
in this stream's track set, regardless of kind.
</summary>
<returns>A Vector of <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> objects representing all the tracks in this stream.</returns>
<seealso cref="M:webrtc_winrt_api.MediaStream.GetAudioTracks"/>
<seealso cref="M:webrtc_winrt_api.MediaStream.GetVideoTracks"/>
</member>
<member name="M:webrtc_winrt_api.MediaStream.GetTrackById(System.String)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="148">
<summary>
Return either a <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> object from this stream's track set whose id is 
equal to trackId, or nullptr, if no such track exists.
</summary>
<param name="trackId">The identifier of the track to return</param>
<returns>A <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> object from this stream's track set whose id is 
equal to trackId, or nullptr, if no such track exists.</returns>
</member>
<member name="M:webrtc_winrt_api.MediaStream.AddTrack(webrtc_winrt_api.IMediaStreamTrack)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="157">
<summary>
Adds the given <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> to this <see cref="T:webrtc_winrt_api.MediaStream"/>.
</summary>
<param name="track"></param>
</member>
<member name="M:webrtc_winrt_api.MediaStream.RemoveTrack(webrtc_winrt_api.IMediaStreamTrack)" decl="true" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="163">
<summary>
Removes the given <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> from this <see cref="T:webrtc_winrt_api.MediaStream"/>.
</summary>
<param name="track"></param>
</member>
<member name="P:webrtc_winrt_api.MediaStream.Id" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="169">
<summary>
Identifier
</summary>
</member>
<member name="P:webrtc_winrt_api.MediaStream.Active" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\media.h" line="176">
<summary>
This attribute is true if the <see cref="T:webrtc_winrt_api.MediaStream"/> has at least one <see cref="T:webrtc_winrt_api.IMediaStreamTrack"/> 
that has not ended, and false otherwise.
</summary>
</member>
<member name="T:webrtc_winrt_api.globals.FileLogSink" decl="false" source="c:\work\webrtc\repositories\webrtc\src\webrtc\build\winrt_gyp\api\peerconnectioninterface.cc" line="70">
a private class only used in this file, which implements LogSink for logging to file

</member>
</members>
</doc>