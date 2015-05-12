// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkID=392286
(function () {
    "use strict";

    var app = WinJS.Application;
    var activation = Windows.ApplicationModel.Activation;
    var renderWindow;
    var remoteRenderWindow
    var mediaSource
    var initialize 

    app.onactivated = function (args) {
        if (args.detail.kind === activation.ActivationKind.launch) {
            if (args.detail.previousExecutionState !== activation.ApplicationExecutionState.terminated) {
                // TODO: This application has been newly launched. Initialize
                // your application here.
            } else {
                // TODO: This application has been reactivated from suspension.
                // Restore application state here.
            }
            args.setPromise(WinJS.UI.processAll().done(function () {

              var button = document.getElementById("#startButton");

              button.addEventListener("click", startButtonClicked, false);

              var attachButton = document.getElementById("#attachButton");

              attachButton.addEventListener("click", attachButtonClicked, false);

              var stopButton = document.getElementById("#stopButton");

              stopButton.addEventListener("click", stopButtonClicked, false);

              webrtc_winjs_api.WinJSMediaEngine.instance.onLaunched();

            }));

            renderWindow = document.getElementById("#locaMediaWindow");
            //renderWindow.removeAttribute("controls");

            remoteRenderWindow = document.getElementById("#remoteMediaWindow");
            //remoteRenderWindow.removeAttribute("controls");


        }
    };

    app.oncheckpoint = function (args) {
        // TODO: This application is about to be suspended. Save any state
        // that needs to persist across suspensions here. You might use the
        // WinJS.Application.sessionState object, which is automatically
        // saved and restored across suspension. If you need to complete an
        // asynchronous operation before your application is suspended, call
        // args.setPromise().
    };
    
    function startButtonClicked(event) {

      webrtc_winjs_api.WinJSMediaEngine.instance.start().then(function () {

        //TODO: fix this.
        //need to remove attach button after asynchronous operations are fixed.
 
      });

    }


    function attachButtonClicked(event) {

        var locaMediaSS = webrtc_winjs_api.WinJSMediaEngine.instance.getLocalMediaStreamSource();

        if (locaMediaSS == null)
          return;

        renderWindow.src = URL.createObjectURL(webrtc_winjs_api.WinJSMediaEngine.instance.getLocalMediaStreamSource());
        renderWindow.autoplay = "autoplay"

        remoteRenderWindow.src = URL.createObjectURL(webrtc_winjs_api.WinJSMediaEngine.instance.getRemoteMediaStreamSource());
        remoteRenderWindow.autoplay = "autoplay"

    }

    function stopButtonClicked(event) {
      //ToDo: it will crash
      // webrtc_winjs_api.WinJSMediaEngine.instance.stop();


    }


    app.start();
})();