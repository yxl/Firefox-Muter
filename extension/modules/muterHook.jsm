let EXPORTED_SYMBOLS = ['muterHook'];

Components.utils.import("resource://gre/modules/ctypes.jsm");
Components.utils.import("resource://muter/muterUtils.jsm");

let isWin7OrLater = muterUtils.platform.isWin7OrLater();

let muterHook = {
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
    
    let dllFileName = isWin7OrLater ? 'MuterWin7.dll' : 'MuterHook.dll';    
    let uri = muterUtils.Services.io.newURI('resource://muter-binary/' + dllFileName, null, null);    
    if (uri instanceof Components.interfaces.nsIFileURL) {
      this._dllFile = ctypes.open(uri.file.path);
    } else {
      return;
    }
    
    let abiType = muterUtils.isVersionLessThan("4.0") ? ctypes.stdcall_abi: ctypes.winapi_abi;

    // void EnableMute(BOOL bEnabeled)
    this._EnableMute = this._dllFile.declare("EnableMute",
      abiType,
      ctypes.void_t, // void
      ctypes.int32_t  //BOOL
      );  
      
    // BOOL IsMuteEnabled()
    this._IsMuteEnabled = this._dllFile.declare("IsMuteEnabled",
      abiType,
      ctypes.int32_t
      );

    if (isWin7OrLater) {
      // BOOL Initialize()
      this._Initialize = this._dllFile.declare("Initialize",
        abiType,
        ctypes.int32_t
        );
      
      // void Dispose()
      this._Dispose = this._dllFile.declare("Dispose",
        abiType,
        ctypes.void_t
        );
      
      this._Initialize();
    }
  },
  
  close: function() {
    if (isWin7OrLater()) {
      this._Dispose();
    }
    if (this._dllFile) {
      this._dllFile.close();
    }
  },
   
  enableMute: function(isEnabled) {
    let enableParam = isEnabled ? 1 : 0;
    this._EnableMute(enableParam);
    muterUtils.Services.obs.notifyObservers(null, "muter-status-changed", null);
  },
  
  isMuteEnabled: function() {
    return this._IsMuteEnabled();
  },
};
