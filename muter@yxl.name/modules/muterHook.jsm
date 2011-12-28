var EXPORTED_SYMBOLS = ['muterHook'];

var jsm = {};

Components.utils.import("resource://gre/modules/Services.jsm", jsm);
Components.utils.import("resource://gre/modules/ctypes.jsm", jsm);

var observerService = Components.classes["@mozilla.org/observer-service;1"]
                  .getService(Components.interfaces.nsIObserverService);

var platform = {
  name: null,
  version: 0.0,
  win: false,
  mac: false,
  linux: false,
  other: false,
  
  isWin7OrLater: function() {
    if (this.win && this.version >= 6.1) {
      return true;
    } else {
      return false;
    }
  },
};

(function checkOSVersion() {  
  // For how to get OS version, refers to https://developer.mozilla.org/en/Code_snippets/Miscellaneous
  let osName = Components.classes["@mozilla.org/xre/app-info;1"]  
               .getService(Components.interfaces.nsIXULRuntime).OS;
  osName = osName.trim().toLowerCase();
  if (osName === 'winnt') {
    platform.name = 'win';
    platform.win = true;
  } else if (osName === 'linux') {
    platform.name = 'linux';
    platform.linux = tue;
  } else if (osName === 'darwin') {
    platform.name = 'mac';
    platform.mac = true;
  } else {
    platform.name = osName;
    platform.other = true;
  }
  let osVersion = Components.classes["@mozilla.org/network/protocol;1?name=http"] 
          .getService(Components.interfaces.nsIHttpProtocolHandler).oscpu;
  var m = /.* (\d+\.\d+)/.exec(osVersion);
  if (m) {
    platform.version = parseFloat(m[1]);
  }
})();

var muterHook = {
  _EnableMute:    null,
  _IsMuteEnabled:  null,
  _Initialize: null,  // win7 only
  _Dispose: null,  // win7 only
 
  _dllFile:          null,

  open: function() {
  
    // Check if it is already opened
    if (this._dllFile) {
      return;
    }
    
    let isWin7OrLater = platform.isWin7OrLater();
    
    let dllFileName = isWin7OrLater ? 'MuterWin7.dll' : 'MuterHook.dll';    
    let uri = jsm.Services.io.newURI('resource://muter-binary/' + dllFileName, null, null);    
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
      jsm.ctypes.int32_t
      );

    if (isWin7OrLater) {
      // BOOL Initialize()
      this._Initialize = this._dllFile.declare("Initialize",
        jsm.ctypes.winapi_abi,
        jsm.ctypes.int32_t
        );
      
      // void Dispose()
      this._Dispose = this._dllFile.declare("Dispose",
        jsm.ctypes.winapi_abi,
        jsm.ctypes.void_t
        );
      
      this._Initialize();
    }
  },
  
  close: function() {
    if (platform.isWin7OrLater()) {
      this._Dispose();
    }
    if (this._dllFile) {
      this._dllFile.close();
    }
  },
   
  enableMute: function(isEnabled) {
    var enableParam = isEnabled ? 1 : 0;
    this._EnableMute(enableParam);
    observerService.notifyObservers(null, "muter-status-changed", null);
  },
  
  isMuteEnabled: function() {
    return this._IsMuteEnabled();
  }
};
