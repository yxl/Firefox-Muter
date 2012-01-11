let EXPORTED_SYMBOLS = ['muterUtils'];

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

let muterUtils = {};

try {
  // Firefox 4+
  Components.utils.import("resource://gre/modules/Services.jsm", muterUtils);
} catch(ex) {
  // Firefox 3.6
  Components.utils.import("resource://muter/Services.jsm", muterUtils);
}

muterUtils.isVersionLessThan = function(v) {
  return (muterUtils.Services.vc.compare(muterUtils.Services.appinfo.version, v) < 0);
}

let platform = {
  name: null,
  version: 0.0,
  is64: false, 
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
  let runtime = muterUtils.Services.appinfo;
  
  // OS Name
  let osName = runtime.OS.trim().toLowerCase();
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
  
  // Platfrom
  if (runtime.XPCOMABI.indexOf('64') == -1) {
    platform.is64 = false;
  } else {
    platform.is64 = true;
  }
  // OS version
  let osVersion = Components.classes["@mozilla.org/network/protocol;1?name=http"] 
          .getService(Components.interfaces.nsIHttpProtocolHandler).oscpu;
  var m = /.* (\d+\.\d+)/.exec(osVersion);
  if (m) {
    platform.version = parseFloat(m[1]);
  }
})();

muterUtils.platform = platform;


muterUtils.getLocaleString = function() {
  return muterUtils.Services.prefs.getCharPref('general.useragent.locale');
}

/**
 * Cache of commonly used string bundles.
 */
muterUtils.Strings = {};
[
  ["defaultSkin", "chrome://muter/locale/defaultSkin.properties"],
].forEach(function(aStringBundle) {
  let[name, bundle] = aStringBundle;
  XPCOMUtils.defineLazyGetter(muterUtils.Strings, name, function() {
    return muterUtils.Services.strings.createBundle(bundle);
  });
});



