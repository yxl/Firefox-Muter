let EXPORTED_SYMBOLS = ['muterUtils'];

let muterUtils = {};

muterUtils.isVersionLessThan = function(v) {
  let appInfo = Components.classes["@mozilla.org/xre/app-info;1"].getService(Components.interfaces.nsIXULAppInfo);
  let versionChecker = Components.classes["@mozilla.org/xpcom/version-comparator;1"].getService(Components.interfaces.nsIVersionComparator);
  return (versionChecker.compare(appInfo.version, v) < 0);
}

let platform = {
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
    platform.linux = true;
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

muterUtils.platform = platform;

if (muterUtils.isVersionLessThan("4.0")) {
  Components.utils.import("resource://muter/Services.jsm", muterUtils);
} else {
  Components.utils.import("resource://gre/modules/Services.jsm", muterUtils);
}



