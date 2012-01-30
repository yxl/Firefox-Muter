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
  let {
    muterUtils, muterHook
  } = jsm;
  let AddonManager = null;
  if (!muterUtils.isVersionLessThan("4.0")) {
    Components.utils.import("resource://gre/modules/AddonManager.jsm", jsm);
    AddonManager = jsm.AddonManager;
  }
  let Services = muterUtils.Services;

  var muter = {
    // Monitors the mute status and updates the UI
    _muterObserver: null,

    // Timer to hide the button popup
    _autoHideTimeoutId: null,

    init: function(event) {
      window.removeEventListener("load", muter.init, false);

      let firstRun = Services.prefs.getBoolPref('extensions.firefox-muter.firstRun');
      if (firstRun) {
        muter._firstRun();
        Services.prefs.setBoolPref('extensions.firefox-muter.firstRun', false);
      }

      muter.setupAddonBar();
      muter.setupStatusBar(); // For firefox 3.6 only      
      muter.setupSwitchButton();
      muter.setupShortcut();
      muter._setupPopups();

      muter.updateUI();

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

      if (AddonManager) {
        // Listen for extension uninstalling
        AddonManager.addAddonListener(muter._addonListener);
      }
    },

    destroy: function(event) {
      window.removeEventListener("unload", muter.destroy, false);

      window.removeEventListener("aftercustomization", muter._onAddonBarChanged, false);
      muter._muterObserver.unregister();

      if (AddonManager) {
        AddonManager.removeAddonListener(muter._addonListener);
      }
    },

    /**
     * Initilization after installation
     * It only need to call once.
     */
    _firstRun: function() {
      // For firefox 3.6 only
      if (muterUtils.isVersionLessThan("4.0")) {
        // Show status bar button
        Services.prefs.setBoolPref('extensions.firefox-muter.showInStatusBar', true);
      }
      muterSkin.ui.updateFromWeb();
    },

    _addonListener: {
      onUninstalling: function(addon) {
        if (addon.id == "muter@yxl.name") {
          muter.uninstall();
        }
      },

      onDisabling: function(addon, needsRestart) {
        if (addon.id == "muter@yxl.name") {
          muter.uninstall();
        }
      },
      
      onInstalling: function(addon, needsRestart) {
        if (addon.id == "muter@yxl.name") {
          muter.uninstall();
        }      
      }

    },


    uninstall: function() {
      // Clear all user preferences
      Services.prefs.deleteBranch('extensions.firefox-muter.');
      Services.obs.notifyObservers(null, "muter-uninstall", null);      
    },

    switchStatus: function(event) {
      let shouldMute = !muterHook.isMuteEnabled();
      muterHook.enableMute(shouldMute);
      muterSkin.ui.updateFromWeb();
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
        muterSkin.ui.updateFromWeb();
        let btn = event.currentTarget;
        let menu = document.getElementById("muter-switch-button-popup-menu");
        if (btn && menu) {
          menu.openPopup(btn, "after_start", -1, 0, true, false);
        }
        event.preventDefault();
        event.stopPropagation();
      }
    },

    showPopuMenu: function(event) {
      muterSkin.ui.updateFromWeb();
      let btn = document.getElementById("muter-toolbar-palette-button");
      let menu = document.getElementById("muter-switch-button-popup-menu");
      if (btn && menu) {
        menu.openPopup(btn, "after_start", -1, 0, true, false);
      }
        event.preventDefault();
        event.stopPropagation();      
    },

    /** Open Settings dialog */
    openSettingsDialog: function(event) {
      event.stopPropagation();

      // For the usage of window.showModalDialog, refer to 
      // https://developer.mozilla.org/en/DOM/window.showModalDialog
      let url = 'chrome://muter/content/muterSettings.xul';
      window.showModalDialog(url);
    },

    openFeedback: function(event) {
      // For the usage of window.openDialog, refer to 
      // https://developer.mozilla.org/en/DOM/window.openDialog
      let url = 'chrome://muter/content/feedback.xul';
      let name = 'muterFeedback';
      let features = 'chrome,centerscreen';
      window.openDialog(url, name, features, gBrowser.currentURI.spec);
    },

    updateUI: function() {
      let isMuted = muterHook.isMuteEnabled();

      // Changes the button status icon
      let btn = document.getElementById("muter-toolbar-palette-button");
      if (btn) {
        let icon = btn.getAttribute(isMuted ? 'image-enabled' : 'image-disabled');
        btn.setAttribute("image", icon);
      }

      // For firefox 3.6 only
      let statusbarBtn = document.getElementById("muter-statusbar-button");
      if (statusbarBtn) {
        let icon = statusbarBtn.getAttribute(isMuted ? 'image-enabled' : 'image-disabled');
        statusbarBtn.setAttribute("image", icon);
      }
    },

    /** Setup the icon of the switch button*/
    setupSwitchButton: function() {
      let disabledIcon = Services.prefs.getCharPref('extensions.firefox-muter.disabledIcon');
      let enabledIcon = Services.prefs.getCharPref('extensions.firefox-muter.enabledIcon');
      let switchButtonType = Services.prefs.getCharPref('extensions.firefox-muter.switchButtonType');

      let btn = document.getElementById("muter-toolbar-palette-button");
      if (btn) {
        btn.setAttribute('image-disabled', disabledIcon);
        btn.setAttribute('image-enabled', enabledIcon);
        if (switchButtonType !== 'none') {
          btn.setAttribute('type', switchButtonType);
        } else {
          btn.removeAttribute('type');
        }
      }

      // For firefox 3.6 only
      let statusbarBtn = document.getElementById("muter-statusbar-button");
      if (statusbarBtn) {
        statusbarBtn.setAttribute('image-disabled', disabledIcon);
        statusbarBtn.setAttribute('image-enabled', enabledIcon);
      }

      this.updateUI();
    },

    /** Add the muter button to the addon bar or remove it */
    setupAddonBar: function() {
      // For firefox 3.6 only
      if (muterUtils.isVersionLessThan("4.0")) {
        return;
      }

      let showInAddonBar = Services.prefs.getBoolPref('extensions.firefox-muter.showInAddonBar');
      let addonbar = document.getElementById("addon-bar");
      if (!addonbar) return;

      let curSet = addonbar.currentSet.replace(/\s/g, '').split(',');
      let isChanged = false;
      let index = curSet.indexOf("muter-toolbar-palette-button");
      if (showInAddonBar && -1 == index) {
        curSet.push("muter-toolbar-palette-button");
        isChanged = true;
      } else if (!showInAddonBar && -1 != index) {
        curSet.splice(index, 1);
        isChanged = true;
      }
      if (isChanged) {
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

        if (showInAddonBar) {
          this._showIntroPopup();
        }
      }

      // prevent showing two switch buttons
      let showInStatusBar = Services.prefs.getBoolPref("extensions.firefox-muter.showInStatusBar");
      if (showInAddonBar) {
        Services.prefs.setBoolPref("extensions.firefox-muter.showInStatusBar", false);
      }
    },

    /** popup an introduction message */
    _showIntroPopup: function() {
      window.setTimeout(function() {
        let initpanel = document.getElementById("muter-popup-init");
        let btn = document.getElementById("muter-toolbar-palette-button");
        if (initpanel && btn) {
          initpanel.openPopup(btn, "topcenter bottomright");
        }
      }, 3000);
    },

    /** For firefox 3.6 only! Setup the muter button in the status bar**/
    setupStatusBar: function() {
      let showInStatusBar = Services.prefs.getBoolPref("extensions.firefox-muter.showInStatusBar");
      let statusbarBtn = document.getElementById("muter-statusbar-button");
      if (statusbarBtn) {
        statusbarBtn.hidden = !showInStatusBar;
      }

      // prevent showing two switch buttons
      let showInAddonBar = Services.prefs.getBoolPref("extensions.firefox-muter.showInAddonBar");
      if (showInStatusBar) {
        Services.prefs.setBoolPref("extensions.firefox-muter.showInAddonBar", false);
      }

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
    
    useDefaultSkin: function() {
      let defaultDisabledIcon = Services.prefs.getCharPref('extensions.firefox-muter.disabledIcon.default');
      let defaultEnabledIcon = Services.prefs.getCharPref('extensions.firefox-muter.enabledIcon.default');
      if (defaultDisabledIcon && defaultEnabledIcon) {
        Services.prefs.setCharPref('extensions.firefox-muter.disabledIcon', defaultDisabledIcon);
        Services.prefs.setCharPref('extensions.firefox-muter.enabledIcon', defaultEnabledIcon);
      }
    },

    _onToolBarChanged: function(event) {
      Services.prefs.setBoolPref('extensions.firefox-muter.showInAddonBar', muter._isButtonShowInAddonBar());
    },

    _setupPopups: function() {
      // introduction popup
      this._setAutoHide(document.getElementById('muter-popup-init'), 3000);
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
   * Observer monitering the mute status and preferences.
   * If the status is changed, updates the UI of each window.
   */
  function MuterObserver() {};

  MuterObserver.prototype = {
    _branch: null,

    observe: function(subject, topic, data) {
      if (topic === "muter-status-changed") {
        muter.updateUI();
      } else if (topic === "nsPref:changed") {
        let prefName = PREF_BRANCH + data;
        if (prefName.indexOf("shortcut.") != -1) {
          muter.setupShortcut();
        } else if (prefName === "extensions.firefox-muter.showInAddonBar") {
          muter.setupAddonBar();
        } else if (prefName == "extensions.firefox-muter.showInStatusBar") {
          muter.setupStatusBar();
        } else if (prefName == "extensions.firefox-muter.disabledIcon" || prefName == "extensions.firefox-muter.enabledIcon" || prefName == "extensions.firefox-muter.switchButtonType") {
          muter.setupSwitchButton();
        } else if (prefName == "extensions.firefox-muter.disabledIcon.default" ||
                   prefName == "extensions.firefox-muter.enabledIcon.default") {
          muter.useDefaultSkin();
          muterSkin.ui.rebuildSkinMenu();
        }
      } else if (topic === "em-action-requested") {
        // For firefox 3.6 only
        subject.QueryInterface(Components.interfaces.nsIUpdateItem);
        if (subject.id === "muter@yxl.name" && (data === "item-disabled" || data === "item-uninstalled" || data === "item-upgraded")) {
          muter.uninstall();
        }
      }
    },

    register: function() {
      Services.obs.addObserver(this, "muter-status-changed", false);

      this._branch = Services.prefs.getBranch(PREF_BRANCH);
      if (this._branch) {
        // Now we queue the interface called nsIPrefBranch2. This interface is described as: 
        // "nsIPrefBranch2 allows clients to observe changes to pref values."
        this._branch.QueryInterface(Components.interfaces.nsIPrefBranch2);
        this._branch.addObserver("", this, false);
      }

      // For firefox 3.6 only!
      Services.obs.addObserver(this, "em-action-requested", false);
    },

    unregister: function() {
      Services.obs.removeObserver(this, "em-action-requested");

      Services.obs.removeObserver(this, "muter-status-changed");

      if (this.branch) {
        this._branch.removeObserver("", this);
      }
    }
  };

  window.addEventListener("load", muter.init, false);
  window.addEventListener("unload", muter.destroy, false);
  return muter;
})();