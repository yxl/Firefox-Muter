/* ***** BEGIN LICENSE BLOCK *****
 * This Source Code is subject to the terms of the Mozilla Public License
 * version 2.0 (the "License"). You can obtain a copy of the License at
 * http://mozilla.org/MPL/2.0/.
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

var muter = (function() {
  let jsm = {};
  Components.utils.import("resource://muter/muterUtils.jsm", jsm);
  Components.utils.import("resource://muter/muterHook.jsm", jsm);
  Components.utils.import("resource://gre/modules/Services.jsm", jsm);
  Components.utils.import("resource://gre/modules/AddonManager.jsm", jsm);
  let {
    muterUtils, muterHook, Services, AddonManager
  } = jsm;

  var muter = {
    // Monitors the mute status and updates the UI
    _muterObserver: null,

    init: function(event) {
      window.removeEventListener("load", muter.init, false);

      muterHook.open();

      let firstRun = Services.prefs.getBoolPref('extensions.firefox-muter.firstRun');
      if (firstRun) {
        muter.showButton();
        Services.prefs.setBoolPref('extensions.firefox-muter.firstRun', false);
      }

      muter.updateUI();
      muter.setupShortcut();


      window.addEventListener("aftercustomization", muter._onToolBarChanged, false);
      muter._onToolBarChanged();

      muter._muterObserver = new MuterObserver();
      muter._muterObserver.register();

      // restore mute status if necessary
      let needRetoreMuteStatus = Services.prefs.getBoolPref('extensions.firefox-muter.saveStatus');
      if (needRetoreMuteStatus) {
        let needMute = Services.prefs.getBoolPref('extensions.firefox-muter.muteStatus');
        muterHook.enableMute(needMute);
      }
    },

    destroy: function(event) {
      window.removeEventListener("aftercustomization", muter._onAddonBarChanged, false);
      muterHook.close();
    },

    uninstall: function() {
      // Clear all user preferences
      Services.prefs.deleteBranch('extensions.firefox-muter.');
    },

    switchStatus: function(event) {
      let shouldMute = !muterHook.isMuteEnabled();
      muterHook.enableMute(shouldMute);
      Services.prefs.setBoolPref('extensions.firefox-muter.muteStatus', shouldMute);
    },

    clickSwitchButton: function(event) {
      if (event.button == 0) {
        // Left button click
        if (event.currentTarget.id === 'muter-statusbar-button') {
          muter.switchStatus();
        }
      } else if (event.button == 2) {
        // Right click to show the popup menu
        let btn = event.currentTarget;
        let menu = document.getElementById("muter-switch-button-popup-menu");
        if (btn && menu) {
          menu.openPopup(btn, "after_start", -1, 0, true, false);
        }
        event.preventDefault();
        event.stopPropagation();
      }
    },


    /** Open Settings dialog */
    openSettingsDialog: function(event) {
      event.stopPropagation();

      // For the usage of window.showModalDialog, refer to
      // https://developer.mozilla.org/en/DOM/window.showModalDialog
      let url = 'chrome://muter/content/muterSettings.xul';
      window.showModalDialog(url);
    },

    updateUI: function() {
      let isMuted = muterHook.isMuteEnabled();

      // Changes the button status icon
      var image_disabled="chrome://muter/skin/mute-disabled.png";
      var image_enabled="chrome://muter/skin/mute-enabled.png";

      let btn = document.getElementById("muter-toolbar-palette-button");
      if (btn) {
        let icon = isMuted ? image_enabled : image_disabled;
        btn.setAttribute("image", icon);
      }

      // For SeaMonkey only
      let statusbarBtn = document.getElementById("muter-statusbar-button");
      if (statusbarBtn) {
        let icon = statusbarBtn.getAttribute(isMuted ? 'image-enabled' : 'image-disabled');
        statusbarBtn.setAttribute("image", icon);
      }
    },

    /** Add the muter button to the addon bar or remove it */
    showButton: function() {
      let addonbar = document.getElementById("addon-bar");
      if (!addonbar)
        return;

      let curSet = addonbar.currentSet.replace(/\s/g, '').split(',');
      let index = curSet.indexOf("muter-toolbar-palette-button");
      if (-1 == index) {
        curSet.push("muter-toolbar-palette-button");
      }
      let newSetString = curSet.join();

      addonbar.currentSet = newSetString;
      addonbar.setAttribute("currentset", newSetString);

      document.persist(addonbar.id, "currentset");
      try {
        BrowserToolboxCustomizeDone(true);
      } catch (e) {}
      if (addonbar.getAttribute("collapsed") == "true") {
        addonbar.setAttribute("collapsed", "false");
      }
      document.persist(addonbar.id, "collapsed");
    },

    setupShortcut: function() {
      try {
        let keyItem = document.getElementById('key_muterToggle');
        if (keyItem) {
          // Default key is "M"
          keyItem.setAttribute('key', Services.prefs.getCharPref('extensions.firefox-muter.shortcut.key'));
          // Default modifiers is "control alt"
          keyItem.setAttribute('modifiers', Services.prefs.getCharPref('extensions.firefox-muter.shortcut.modifiers'));
        }
      } catch (e) {}
    },

    _onToolBarChanged: function(event) {
      Services.prefs.setBoolPref('extensions.firefox-muter.showInAddonBar', muter._isButtonShowInAddonBar());
    },

    _isButtonShowInAddonBar: function() {
      let addonbar = document.getElementById("addon-bar");
      if (addonbar) {
        let curSet = addonbar.currentSet;
        return curSet.indexOf("muter-toolbar-palette-button") != -1;
      }
      return false;
    }
  };

  const PREF_BRANCH = "extensions.firefox-muter.";
  /**
   * Observer monitering the mute status, preferences and addon disable/uninstall.
   * If the status is changed, updates the UI of each window.
   */
  function MuterObserver() {};

  MuterObserver.prototype = {
    _branch: null,

    observe: function(subject, topic, data) {
      if (topic === "muter-status-changed") {
        muter.updateUI();
      } else if (topic === "nsPref:changed") {
        if (prefName.indexOf("shortcut.") != -1) {
          muter.setupShortcut();
        }
      }
    },

    register: function() {
      Services.obs.addObserver(this, "muter-status-changed", false);
      this._branch = Services.prefs.getBranch(PREF_BRANCH);
      if (this._branch) {
        // nsIPrefBranch2 has been merged into nsIPrefBranch in Gecko 13. Once we drop support for old versions of Gecko, we should stop using nsIPrefBranch2.
        try {
          this._branch.QueryInterface(Components.interfaces.nsIPrefBranch2);
        } catch (ex) {
        }
        this._branch.addObserver("", this, false);
      }

    },

    unregister: function() {
      Services.obs.removeObserver(this, "muter-status-changed");
      if (this._branch) {
        this._branch.removeObserver("", this);
      }
    },

  };

  window.addEventListener("load", muter.init, false);
  window.addEventListener("unload", muter.destroy, false);
  return muter;
})();