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

    // Timer to hide the button popup
    _autoHideTimeoutId: 0,
    
    _isVolumeControlShown: false,
    
    _hideVolumeControlTimeoutId: 0,

    init: function(event) {
      window.removeEventListener("load", muter.init, false);

      muterHook.open();

      let firstRun = Services.prefs.getBoolPref('extensions.firefox-muter.firstRun');
      if (firstRun) {
        muter._firstRun();
        Services.prefs.setBoolPref('extensions.firefox-muter.firstRun', false);
      }

      muter.setupAddonBar();
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
    },

    destroy: function(event) {
      window.removeEventListener("unload", muter.destroy, false);

      window.removeEventListener("aftercustomization", muter._onAddonBarChanged, false);
      muter._muterObserver.unregister();

      muterHook.close();
    },

    /**
     * Initilization after installation
     * It only needs to call once.
     */
    _firstRun: function() {
      muterSkin.ui.updateFromWeb();
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
      this._isVolumeControlShown = false;
      if (event.button == 0) {
        // Left button click
        muterSkin.ui.updateFromWeb();
        this._showVolumeControl();
      } else if (event.button == 2) {
        // Right click to show the popup menu
        let btn = event.currentTarget;
        let menu = document.getElementById("muter-switch-button-popup-menu");
        if (btn && menu) {
          menu.openPopup(btn, "after_start", -1, 0, false, false);
        }
        event.preventDefault();
        event.stopPropagation();
      }
    },

    onMouseOverSwitchButton: function(event) {
      if (this._hideVolumeControlTimeoutId) {
        clearTimeout(this._hideVolumeControlTimeoutId);
        this._hideVolumeControlTimeoutId = 0;
      }
      this._showVolumeControl();
    },

    onMouseOutSwitchButton: function(event) {
      let btn = document.getElementById("muter-toolbar-palette-button");
      let rect = btn.getBoundingClientRect();
      let isMouseInControls = event.clientY > rect.top &&
                              event.clientY < rect.bottom &&
                              event.clientX > rect.left &&
                              event.clientX < rect.right;
      
      if (!isMouseInControls) {
        let panel = document.getElementById("muter-popup-volume");
        panel = btn.getBoundingClientRect();
        isMouseInControls = event.clientY > rect.top &&
                                event.clientY < rect.bottom &&
                                event.clientX > rect.left &&
                                event.clientX < rect.right;
      }

      if (this._hideVolumeControlTimeoutId) {
        clearTimeout(this._hideVolumeControlTimeoutId);
      }
      if (!isMouseInControls) {
        this._hideVolumeControlTimeoutId = setTimeout(this._hideVolumeControl.bind(this), 200);
      }
    },

    onMouseOutVolumeControl: function(event) {
      let panel = document.getElementById("muter-popup-volume");
      let rect = panel.getBoundingClientRect();
      let isMouseInControls = event.clientY > rect.top &&
                              event.clientY < rect.bottom &&
                              event.clientX > rect.left &&
                              event.clientX < rect.right;

      if (this._hideVolumeControlTimeoutId) {
        clearTimeout(this._hideVolumeControlTimeoutId);
      }
      if (!isMouseInControls) {
        this._hideVolumeControlTimeoutId = setTimeout(this._hideVolumeControl.bind(this), 100);
      }
    },
    
    onVolumeControlChange: function() {
      let scale = document.getElementById("muter-volume-control");
      let volume = parseInt(scale.value);
      let isMuted = muterHook.isMuteEnabled();
      if (isMuted && volume == 0) {
        return;
      }
      if (muterHook.getVolume() != volume) {
        muterHook.setVolume(volume);
      }
      if (isMuted) {
        muterHook.enableMute(false);
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
      
      let scale = document.getElementById("muter-volume-control");
      if (scale) {
        scale.value = isMuted ? 0 : muterHook.getVolume();
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

      this.updateUI();
    },

    /** Add the muter button to the addon bar or remove it */
    setupAddonBar: function() {
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

    /** popup volume control */
    _showVolumeControl: function() {
      if (this._isVolumeControlShown) {
        return;
      }
      this._isVolumeControlShown = true;
      let stack = document.getElementById("muter-volume-stack");
      stack.setAttribute("fadeout", true);
      let panel = document.getElementById("muter-popup-volume");
      let btn = document.getElementById("muter-toolbar-palette-button");
      if (panel && btn) {
        panel.openPopup(btn, "before_start", 0, 0, true, false);
      }
      window.setTimeout(function() {
        stack.removeAttribute("fadeout");
      }, 100);
    },
    
    _hideVolumeControl: function() {
      if (!this._isVolumeControlShown) {
        return;
      }
      this._isVolumeControlShown = false;
      let panel = document.getElementById("muter-popup-volume");
      if (panel) {
        panel.hidePopup();
      }
      let stack = document.getElementById("muter-volume-stack");
      stack.setAttribute("fadeout", true);
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
        let prefName = PREF_BRANCH + data;
        if (prefName.indexOf("shortcut.") != -1) {
          muter.setupShortcut();
        } else if (prefName === "extensions.firefox-muter.showInAddonBar") {
          muter.setupAddonBar();
        } else if (prefName == "extensions.firefox-muter.disabledIcon" || prefName == "extensions.firefox-muter.enabledIcon" || prefName == "extensions.firefox-muter.switchButtonType") {
          muter.setupSwitchButton();
        } else if (prefName == "extensions.firefox-muter.disabledIcon.default" ||
                   prefName == "extensions.firefox-muter.enabledIcon.default") {
          muter.useDefaultSkin();
          muterSkin.ui.rebuildSkinMenu();
        }
      }
    },

    register: function() {
      Services.obs.addObserver(this, "muter-status-changed", false);

      this._branch = Services.prefs.getBranch(PREF_BRANCH);
      if (this._branch) {
        this._branch.addObserver("", this, false);
      }

      // Listen for extension disable/uninstall
      AddonManager.addAddonListener(this._addonListener);
    },

    unregister: function() {
      AddonManager.removeAddonListener(this._addonListener);

      Services.obs.removeObserver(this, "muter-status-changed");

      if (this._branch) {
        this._branch.removeObserver("", this);
      }
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
      }
    }
  };

  window.addEventListener("load", muter.init, false);
  window.addEventListener("unload", muter.destroy, false);
  return muter;
})();