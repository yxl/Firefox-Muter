/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Firefox Muter.
 *
 * The Initial Developer of the Original Code is
 * the Mozilla Online.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Yuan Xulei <xyuan@mozilla.com>
 *  Hector Zhao <bzhao@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

var muter = (function(){
  let jsm = {};
  Components.utils["import"]("resource://gre/modules/Services.jsm", jsm);
  Components.utils["import"]("resource://muter/muterHook.jsm", jsm);
  let { Services, muterHook } = jsm;

  var muter = {
    // Monitors the mute status and updates the UI
    _muterObserver: null,
    
    // Timer to hide the button popup
    _autoHideTimeoutId: null,

    init: function(event) {
      window.removeEventListener("load", muter.init, false);
      muter._setupAddonBar();
      muter._setupShortcut();
      muter._setupPopups();
      
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
    
    updateUI: function() {
      let isMuted = muterHook.isMuteEnabled();

      // Changes the button status icon
      let btn = document.getElementById("muter-toolbar-palette-button");
      if (btn) {
        btn.setAttribute("mute", (isMuted ? "enabled" : "disabled"));
      }

      // Changes the button popup content
      let popupMuteEnabled = document.getElementById("muter-popup-mute-enabled");
      if (popupMuteEnabled) {
        popupMuteEnabled.hidden = !isMuted;
      }
      let popupMuteDisabled = document.getElementById("muter-popup-mute-disabled");
      if (popupMuteDisabled) {
        popupMuteDisabled.hidden = isMuted;
      }      
    },

    /** Move the muter button to the addon bar */
    _setupAddonBar: function() {
      // Check if we have already made it.
      var buttonInstalled = Services.prefs.getBoolPref('extensions.firefox-muter.button-installed');
      if (buttonInstalled) {
        return;
      }
 
      // Move the muter button to the addon bar
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
      }
      
      Services.prefs.setBoolPref('extensions.firefox-muter.button-installed', true);
    },

    _setupShortcut: function() {
      try {
        document.getElementById('key_muterToggle').setAttribute('key', Services.prefs.getCharPref('extensions.firefox-muter.shortcut.key'));
        document.getElementById('key_muterToggle').setAttribute('modifiers', Services.prefs.getCharPref('extensions.firefox-muter.shortcut.modifiers'));
      } catch(e) {}
    },
    
    _setupPopups: function() {
      // muter introduction popup
      this._setAutoHide(document.getElementById('muter-popup-init'), 3000);
      
      // muter button popup
      this._setAutoHide(document.getElementById('muter-popup'), 2000);
    },
    
    _setAutoHide: function(panel, timeout) {
      let self = this;
      panel.addEventListener('popupshown', function() {
        self._autoHideTimeoutId = window.setTimeout(function() {
          panel.hidePopup();
        }, timeout);
      }, false);

      panel.addEventListener('popuphidden', function() {
        window.clearTimeout(self._autoHideTimeoutId);
      }, false);
    }
  };

  /**
   * Observer monitering the mute status.
   * If the status is changed, updates the UI of each window.
   */
  function MuterObserver()  {
  };

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
  };

  window.addEventListener("load", muter.init, false);
  window.addEventListener("unload", muter.destroy, false);
  return muter;
})();
