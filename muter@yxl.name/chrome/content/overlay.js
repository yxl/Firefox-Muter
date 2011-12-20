/**
 * @author  Yuan Xulei <hi@yxl.name>
 */
 
Components.utils["import"]("resource://muter/muterHook.jsm");

var muter = {
  _muterObserver: null,

  init: function(event) {
    window.removeEventListener("load", muter.init, false);
    muter.setupAddonBar();
    
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
    
    // Changes the button status icon
    let btn = document.getElementById("muter-toolbar-palette-button");
    if (btn) {
      btn.setAttribute("mute", (shouldMute ? "enabled" : "disabled"));
    }
    let panel = document.getElementById("muter-popup");
    if (panel) {
      panel.setAttribute("mute", (shouldMute ? "enabled" : "disabled"));
    }
  },
  
  /** Add the muter button to the addon bar */
  setupAddonBar: function() {
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
    }
  },
  
  updateUI: function() {
    let isMuted = muterHook.isMuteEnabled();
    
    // Changes the button status icon
    let btn = document.getElementById("muter-toolbar-palette-button");
    if (btn) {
      btn.setAttribute("mute", (isMuted ? "enabled" : "disabled"));
    }
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
    var observerService = Components.classes["@mozilla.org/observer-service;1"]  
                          .getService(Components.interfaces.nsIObserverService);  
    observerService.addObserver(this, "muter-status-changed", false);  
  },  
  
  unregister: function() {  
    var observerService = Components.classes["@mozilla.org/observer-service;1"]  
                            .getService(Components.interfaces.nsIObserverService);  
    observerService.removeObserver(this, "muter-status-changed");  
  }  
} 

window.addEventListener("load", muter.init, false);
window.addEventListener("unload", muter.destroy, false);
