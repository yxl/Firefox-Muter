let EXPORTED_SYMBOLS = ['muterHook'];

Components.utils.import("resource://muter/muterUtils.jsm");
Components.utils.import("resource://gre/modules/ctypes.jsm");
Components.utils.import("resource://muter/Services.jsm");

let isWin7OrLater = muterUtils.platform.isWin7OrLater();

let muterHook = {
  _EnableMute:    null,
  _IsMuteEnabled:  null,
  _Initialize: null,  // win7 only
  _Dispose: null,  // win7 only
  _refCount: 0,
 
  _lib:          null,

  open: function() {
    this._refCount++;
    // Check if it is already opened
    if (this._lib) {
      return;
    }
    
    let libFile = '';
    let suffix = muterUtils.platform.is64 ? '-64' : '-32';
    if (muterUtils.platform.win) {
       libFile = isWin7OrLater ? 'MuterWin7' : 'MuterHook';
       libFile += suffix + '.dll';       
    } else if (muterUtils.platform.linux) {
       libFile = 'libMuterPulseAudio';
       libFile += suffix + '.so';
    }
    let uri = Services.io.newURI('resource://muter-binary/' + libFile, null, null);    
    if (uri instanceof Components.interfaces.nsIFileURL) {
      this._lib = ctypes.open(uri.file.path);
    } else {
      return;
    }
    
    let abiType = ctypes.default_abi;
    if (muterUtils.platform.win) {
      abiType = ctypes.winapi_abi;
    }

    // void EnableMute(BOOL bEnabeled)
    this._EnableMute = this._lib.declare("EnableMute",
      abiType,
      ctypes.void_t, // void
      ctypes.int32_t  //BOOL
      );  
      
    // BOOL IsMuteEnabled()
    this._IsMuteEnabled = this._lib.declare("IsMuteEnabled",
      abiType,
      ctypes.int32_t
      );

    if (isWin7OrLater || muterUtils.platform.linux) {
      // BOOL Initialize()
      this._Initialize = this._lib.declare("Initialize",
        abiType,
        ctypes.int32_t
        );
      
      // void Dispose()
      this._Dispose = this._lib.declare("Dispose",
        abiType,
        ctypes.void_t
        );
      
      this._Initialize();
    }
  },
  
  close: function() {
    if (--this._refCount > 0)
      return;
    
    if (!this._lib) {
      return;
    }
    if (isWin7OrLater || muterUtils.platform.linux) {
      this._Dispose();
    }
    this._lib.close();
    
    this._lib = null;
  },
   
  enableMute: function(isEnabled) {
    if (!this._lib) {
      return;
    }
    let enableParam = isEnabled ? 1 : 0;
    this._EnableMute(enableParam);
    Services.obs.notifyObservers(null, "muter-status-changed", null);
  },
  
  isMuteEnabled: function() {
    if (!this._lib) {
      return false;
    }
    return this._IsMuteEnabled();
  }
};