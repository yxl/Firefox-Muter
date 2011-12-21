/**
 * @author  Yuan Xulei <hi@yxl.name>
 */
var muter = (function(){
  let jsm = {};
  Components.utils["import"]("resource://gre/modules/Services.jsm", jsm);
  Components.utils["import"]("resource://muter/muterHook.jsm", jsm);
  let { Services, muterHook } = jsm;

  var muter = {
    _muterObserver: null,
    _autoHideTimeoutId: null,

    init: function(event) {
      window.removeEventListener("load", muter.init, false);
      muter.setupAddonBar();
      muter.setAutoHide(document.getElementById('muter-popup-init'), 3000);
      muter.setAutoHide(document.getElementById('muter-popup'), 3000);
      
      muter.updateUI();

      muter._muterObserver = new MuterObserver();
      muter._muterObserver.register();
    },

    destroy: function(event) {
      window.removeEventListener("unload", muter.destroy, false);

      muter._muterObserver.unregister();
    },

    switchStatus: function(event) {
      let shouldMute = !muterHook.isMuteEnabled();
      muterHook.enableMute(shouldMute);
    },

    /** Add the muter button to the addon bar */
    setupAddonBar: function() {
      var buttonInstalled = Services.prefs.getBoolPref('extensions.firefox-muter.button-installed');
      if (buttonInstalled) {
        return;
      }
      var addonbar = window.document.getElementById("addon-bar");
      let curSet = addonbar.currentSet;
      if (-1 == curSet.indexOf("muter-toolbar-palette-button")){
        let newSet = curSet + ",muter-toolbar-palette-button";
        addonbar.currentSet = newSet;
        addonbar.setAttribute("currentset", newSet);
        document.persist(addonbar.id, "currentset");
        try{
          BrowserToolboxCustomizeDone(true);
        }catch(e){}
        if (addonbar.getAttribute("collapsed") == "true") {
          addonbar.setAttribute("collapsed", "false");
        }
        document.persist(addonbar.id, "collapsed");

        let initpanel = document.getElementById("muter-popup-init");
        let btn = document.getElementById("muter-toolbar-palette-button");
        if (initpanel && btn) {
          initpanel.openPopup(btn, "topcenter bottomright");
        }

        Services.prefs.setBoolPref('extensions.firefox-muter.button-installed', true);
      }
    },

    setAutoHide: function(panel, timeout) {
      panel.addEventListener('popupshown', function() {
        muter._autoHideTimeoutId = window.setTimeout(function() {
          panel.hidePopup();
        }, timeout);
      }, false);

      panel.addEventListener('popuphidden', function() {
        window.clearTimeout(muter._autoHideTimeoutId);
      }, false);
    },

    updateUI: function() {
      let isMuted = muterHook.isMuteEnabled();

      // Changes the button status icon
      let btn = document.getElementById("muter-toolbar-palette-button");
      if (btn) {
        btn.setAttribute("mute", (isMuted ? "enabled" : "disabled"));
      }

      // Changes the button popup tips
      let panel = document.getElementById("muter-popup");
      if (panel) {
        panel.setAttribute("mute", (isMuted ? "enabled" : "disabled"));
      }
    }
  }

  function MuterObserver()  {
  }

  MuterObserver.prototype = {
    observe: function(subject, topic, data) {
      muter.updateUI();
    },
    register: function() {
      Services.obs.addObserver(this, "muter-status-changed", false);
    },

    unregister: function() {
      Services.obs.removeObserver(this, "muter-status-changed");
    }
  }

  window.addEventListener("load", muter.init, false);
  window.addEventListener("unload", muter.destroy, false);
  return muter;
})();
