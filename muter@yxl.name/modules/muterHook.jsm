var EXPORTED_SYMBOLS = ['muterHook'];

const Cu = Components.utils;
const Cc = Components.classes;
const Ci = Components.interfaces;

var jsm = {};

Cu.import("resource://gre/modules/Services.jsm", jsm);
Cu.import("resource://gre/modules/ctypes.jsm", jsm);

var muterHook = {
  _EnableMute:    null,
  _IsMuteEnabled:  null,
  _GetLastSoundPlayingTimeInSeconds: null,

  _dllFile:          null,

  open: function() {
    let uri = jsm.Services.io.newURI('resource://muter/MuterHook.dll', null, null);
    if (uri instanceof Components.interfaces.nsIFileURL) {
      this._dllFile = jsm.ctypes.open(uri.file.path);
    } else {
      return;
    }

    // void EnableMute(BOOL bEnabeled)
    this._EnableMute = this._dllFile.declare("EnableMute",
      jsm.ctypes.winapi_abi,
      jsm.ctypes.void_t, // void
      jsm.ctypes.int32_t  //BOOL
      );  
      
    // BOOL IsMuteEnabled()
    this._IsMuteEnabled = this._dllFile.declare("IsMuteEnabled",
      jsm.ctypes.winapi_abi,
      jsm.ctypes.int32_t);
      
    // LONG GetLastSoundPlayingTimeInSeconds()
    this._GetLastSoundPlayingTimeInSeconds = this._dllFile.declare("GetLastSoundPlayingTimeInSeconds",
      jsm.ctypes.winapi_abi,
      jsm.ctypes.int32_t);
  },
  
  close: function() {
    this._dllFile.close();
  },
   
  enableMute: function(isEnabled) {
    var enableParam = isEnabled ? 1 : 0;
    this._EnableMute(enableParam);
  },
  
  isMuteEnabled: function() {
    return this._IsMuteEnabled();
  },
  
  getLastSoundPlayingTimeInSeconds: function() {
    return this._GetLastSoundPlayingTimeInSeconds();
  },
  
  getLastSoundPlayingTime: function() {
    let seconds = this.getLastSoundPlayingTimeInSeconds();
    if (seconds == -1) {
      return null;
    }
    return new Date(seconds * 1000);
  }
};
