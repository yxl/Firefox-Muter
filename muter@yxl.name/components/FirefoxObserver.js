const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;
const Cu = Components.utils;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://muter/muterHook.jsm");

// Gecko 1.9.0/1.9.1 compatibility - add XPCOMUtils.defineLazyServiceGetter
if (!("defineLazyServiceGetter" in XPCOMUtils)) {
	XPCOMUtils.defineLazyServiceGetter = function XPCU_defineLazyServiceGetter(obj, prop, contract, iface) {
		obj.__defineGetter__(prop, function XPCU_serviceGetter() {
			delete obj[prop];
			return obj[prop] = Cc[contract].getService(Ci[iface]);
		});
	};
}

/**
 * Application startup/shutdown observer, triggers init()/shutdown() methods in muterHook.jsm module.
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

	QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver, Ci.nsISupportsWeakReference]),

	observe: function(subject, topic, data) {
		let observerService = Cc["@mozilla.org/observer-service;1"].getService(Ci.nsIObserverService);
		switch (topic) {
		case "profile-after-change":   
      // Refers to https://developer.mozilla.org/en/Observer_Notifications.
      // Loads the hook dll when the firefox starts
      muterHook.open();
			break;
      
    case "quit-application":
      // The application is about to quit. This can be in response to a normal shutdown, or a restart. 
      // Refers to https://developer.mozilla.org/en/Observer_Notifications.
      // Unloads the hook dll when the firefox quits
      muterHook.close();
      break;
		}
	}
};

if (XPCOMUtils.generateNSGetFactory) 
  var NSGetFactory = XPCOMUtils.generateNSGetFactory([FirefoxObserver]);
else
  var NSGetModule = XPCOMUtils.generateNSGetModule([FirefoxObserver]);