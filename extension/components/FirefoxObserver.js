Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

/**
 * Application startup/shutdown observer, triggers open()/close() methods in muterHook.jsm module.
 * @constructor
 */

function FirefoxObserver() {}
FirefoxObserver.prototype = {
  classDescription: "FirefoxObserver",
  contractID: "@yxl.name/muter/startup;1",
  classID: Components.ID("F9F38A8B-99B3-4CBC-87BF-3C9CFF16A013"),
  _xpcom_categories: [{
    category: "app-startup",
    service: true
  }],

  QueryInterface: XPCOMUtils.generateQI([Components.interfaces.nsIObserver, Components.interfaces.nsISupportsWeakReference]),

  observe: function(subject, topic, data) {
    Components.utils.import("resource://muter/muterHook.jsm");
    Components.utils.import("resource://muter/muterUtils.jsm");
    switch (topic) {
    case "app-startup":
      // This is only for firefox 3.6.
      if (muterUtils.isVersionLessThan("4.0")) {
        muterUtils.Services.obs.addObserver(this, "profile-after-change", false);
      }
      break;
    case "profile-after-change":
      // Refers to https://developer.mozilla.org/en/Observer_Notifications.
      // Loads the hook dll when the firefox starts
      muterHook.open();
      muterUtils.Services.obs.addObserver(this, "quit-application", false);
      
      muterUtils.Services.obs.addObserver(this, "muter-uninstall", false);
      break;
    case "quit-application":
      // This is only for firefox 3.6.
      if (muterUtils.isVersionLessThan("4.0")) {
        muterUtils.Services.obs.removeObserver(this, "profile-after-change");
      }
      muterUtils.Services.obs.removeObserver(this, "quit-application");
      // The application is about to quit. This can be in response to a normal shutdown, or a restart.
      // Refers to https://developer.mozilla.org/en/Observer_Notifications.
      // Unloads the hook dll when the firefox quits
      muterHook.close();
      break;
    case "muter-uninstall":
      if (muterUtils.isVersionLessThan("4.0")) {
        muterUtils.Services.obs.removeObserver(this, "profile-after-change");
      }    
      muterUtils.Services.obs.removeObserver(this, "quit-application");
      break;
    }
  }
};

if (XPCOMUtils.generateNSGetFactory) {
  var NSGetFactory = XPCOMUtils.generateNSGetFactory([FirefoxObserver]);
} else {
  var NSGetModule = XPCOMUtils.generateNSGetModule([FirefoxObserver]);
}