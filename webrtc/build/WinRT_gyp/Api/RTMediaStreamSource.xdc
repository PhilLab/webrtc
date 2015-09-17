<?xml version="1.0"?><doc>
<members>
<member name="M:webrtc.IceCandidateInterface.sdp_mid" decl="false" source="c:\work\webrtc\repositories\webrtc\src\talk\app\webrtc\jsep.h" line="60">
If present, this contains the identierfier of the "media stream
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
</members>
</doc>