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
var muterSettings = (function() {
  let jsm = {};
  Components.utils.import("resource://muter/muterUtils.jsm", jsm);
  Components.utils.import("resource://gre/modules/Services.jsm", jsm);
  let {
    muterUtils, Services
  } = jsm;

  let muterSettings = {

    init: function(event) {
      // For SeaMonkey
      if (Services.appinfo.name === 'SeaMonkey') {
        let showInAddonBar = document.getElementById("muter-settings-show-in-addon-bar");
        showInAddonBar.hidden = true;
        let switchButtonStype = document.getElementById("muter-settings-switch-button-style");
        switchButtonStype.hidden = true;
      } else {
        let showInStatusBar = document.getElementById("muter-settings-show-in-status-bar");
        showInStatusBar.hidden = true;
      }
      this._updateDefaultIcons();
    },

    destory: function(event) {},

    saveSettings: function(event) {},

    restoreSettings: function(event) {
      let preferences = document.getElementsByTagName("preference");
      for (let i = 0; i < preferences.length; i++) {
        preferences[i].value = preferences[i].defaultValue;
      }
      // For SeaMonkey
      if (Services.appinfo.name === 'SeaMonkey') {
        let showInStatusBarPref = document.getElementById("showInStatusBar");
        showInStatusBarPref.value = true;
      }
      this._updateDefaultIcons();
    },

    openFeedback: function() {
      // For the usage of window.openDialog, refer to
      // https://developer.mozilla.org/en/DOM/window.openDialog
      let url = 'chrome://muter/content/feedback.xul';
      let name = 'muterFeedback';
      let features = 'chrome,centerscreen';
      window.openDialog(url, name, features);
    },

    _updateDefaultIcons: function() {
      let defaultDisabledIcon = document.getElementById('defaultDisabledIcon');
      let disabledIconImage = document.getElementById('muter-settings-default-disabled-icon');
      if (defaultDisabledIcon && disabledIconImage) {
        disabledIconImage.setAttribute("src", defaultDisabledIcon.value);
      }

      let defaultEnabledIcon = document.getElementById('defaultEnabledIcon');
      let enabledIconImage = document.getElementById('muter-settings-default-enabled-icon');
      if (defaultEnabledIcon && enabledIconImage) {
        enabledIconImage.setAttribute("src", defaultEnabledIcon.value);
      }
    },

    changeDisabledIcon: function() {
      let defaultDisabledIcon = document.getElementById('defaultDisabledIcon');
      if (!defaultDisabledIcon) {
        return;
      }

      let fileName = this._getIconFilePath();
      if (fileName) {
        defaultDisabledIcon.value = fileName;
        this._updateDefaultIcons();
      }
    },

    resetDisabledIcon: function() {
      let defaultDisabledIcon = document.getElementById('defaultDisabledIcon');
      if (!defaultDisabledIcon) {
        return;
      }

      defaultDisabledIcon.value = defaultDisabledIcon.defaultValue;
      this._updateDefaultIcons();
    },

    changeEnabledIcon: function() {
      let defaultEnabledIcon = document.getElementById('defaultEnabledIcon');
      if (!defaultEnabledIcon) {
        return;
      }

      let fileName = this._getIconFilePath();
      if (fileName) {
        defaultEnabledIcon.value = fileName;
        this._updateDefaultIcons();
      }
    },

    resetEnabledIcon: function() {
      let defaultEnabledIcon = document.getElementById('defaultEnabledIcon');
      if (!defaultEnabledIcon) {
        return;
      }

      defaultEnabledIcon.value = defaultEnabledIcon.defaultValue;
      this._updateDefaultIcons();
    },

    _getIconFilePath: function() {
      let fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);

      let parentWindow = window;
      let title = "";
      let mode = fp.modeOpen;
      fp.init(parentWindow, title, mode);
      fp.defaultExtension = "png";
      fp.appendFilters(fp.filterImages);

      if (fp.show() != fp.returnCancel) {
        return fp.fileURL.spec;
      }
      return null;
    }
  };

  return muterSettings;
})();