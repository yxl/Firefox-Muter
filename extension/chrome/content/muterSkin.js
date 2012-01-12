/**
 * This Source Code is subject to the terms of the Mozilla Public License
 * version 2.0 (the "License"). You can obtain a copy of the License at
 * http://mozilla.org/MPL/2.0/.
 */
Components.utils.import("resource://muter/muterUtils.jsm");
Components.utils.import("resource://muter/muterHook.jsm");

/**
 * muterSkin namespace.
 */
if (typeof(muterSkin) == "undefined") {
  var muterSkin = {
    init: function() {}
  };

  /**
   * constructor
   */ (function() {
    this.init();
  }).apply(muterSkin);
};

/**
 *　Manage the UI relative to the skin function
 */
muterSkin.ui = {
  _isSkinUpdating: false,

  load: function() {
    window.removeEventListener("load", muterSkin.ui.load, false);

    muterSkin.db.init();
    muterSkin.ui.rebuildSkinMenu();
  },

  unload: function() {
    window.removeEventListener("unload", muterSkin.ui.unload, false);
  },

  /** 
   *  Build the skin selection popup menu
   */
  rebuildSkinMenu: function() {
    let menuPopup = document.getElementById('muter-switch-button-popup-menu');
    let closingSeparator = document.getElementById('muter-skin-menu-separator-closing');

    // Remove everything above the separator
    while (closingSeparator.previousSibling) {
      menuPopup.removeChild(closingSeparator.previousSibling);
    }

    let locale = muterUtils.getLocaleString();
    // Create skin list
    let data = muterSkin.db.skinArray;
    for (let i = 0; i < data.length; i++) {
      let item = data[i];
      let name = item.name[locale] || '';
      let menuItem = this._createSkinMenuItem(name, item.disabledIconUrl, item.enabledIconUrl);
      menuPopup.insertBefore(menuItem, closingSeparator);
    }
  },

  /**
   * 
   */
  _createSkinMenuItem: function(name, disabledIconUrl, enabledIconUrl) {
    let item = document.createElement("menuitem");

    item.setAttribute("class", "menuitem-iconic");
    item.setAttribute("image", disabledIconUrl);
    item.setAttribute("image-disabled-url", disabledIconUrl);
    item.setAttribute("image-enabled-url", enabledIconUrl);
    item.setAttribute("label", name);
    item.addEventListener("DOMMenuItemActive", function(event) {
      muterSkin.ui._onPreviewSkin(event)
    }, false);
    item.addEventListener("DOMMenuItemInactive", function(event) {
      muterSkin.ui._onResetSkin(event)
    }, false);
    item.setAttribute("oncommand", "muterSkin.ui._onSelectSkin(event)");

    // Download the disabled icon
    let disImg = new Image();
    disImg.onload = function(event) {
      let data = muterSkin.ui._getImageDataURL(this);
      item.setAttribute("image-disabled", data);
    };
    disImg.src = disabledIconUrl;

    // Download the enabled icon
    let enImg = new Image();
    enImg.onload = function(event) {
      let data = muterSkin.ui._getImageDataURL(this);
      item.setAttribute("image-enabled", data);
    };
    enImg.src = enabledIconUrl;

    return item;
  },

  _onPreviewSkin: function(event) {
    let menuItem = event.originalTarget;
    let img = menuItem.getAttribute(muterHook.isMuteEnabled() ? 'image-enabled-url' : 'image-disabled-url');

    let btn = document.getElementById("muter-toolbar-palette-button");
    btn.setAttribute('image', img);

    let btnStatusBar = document.getElementById("muter-statusbar-button");
    btnStatusBar.setAttribute('src', img);
  },

  _onResetSkin: function(event) {
    let btn = document.getElementById("muter-toolbar-palette-button");
    let img = btn.getAttribute(muterHook.isMuteEnabled() ? 'image-enabled' : 'image-disabled');
    btn.setAttribute('image', img);

    let btnStatusBar = document.getElementById("muter-statusbar-button");
    let img = btnStatusBar.getAttribute(muterHook.isMuteEnabled() ? 'image-enabled' : 'image-disabled');
    btnStatusBar.setAttribute('src', img);

  },

  _onSelectSkin: function(event) {
    let menuItem = event.originalTarget;
    let disabledIcon = menuItem.getAttribute('image-disabled');
    let enabledIcon = menuItem.getAttribute('image-enabled');
    // Here we just change the preference. The preference observer defined in overly.js
    // will notify the switch button to change its icon.
    if (disabledIcon && enabledIcon) {
      muterUtils.Services.prefs.setCharPref('extensions.firefox-muter.disabledIcon', disabledIcon);
      muterUtils.Services.prefs.setCharPref('extensions.firefox-muter.enabledIcon', enabledIcon);
    }
    event.stopPropagation();
  },

  /**
   * Convert the image object to base64 encoded data url
   */
  _getImageDataURL: function(img) {
    var canvas = document.createElementNS("http://www.w3.org/1999/xhtml", "html:canvas");
    var context = canvas.getContext("2d");
    canvas.height = img.height;
    canvas.width = img.width;
    context.drawImage(img, 0, 0, img.width, img.height);
    return canvas.toDataURL('image/png', '');
  },

  /**
   * Update the skin list from internet
   */
  updateFromWeb: function() {
    if (this._isSkinUpdating) {
      return;
    }
    this._isSkinUpdating = true;
    let url = muterUtils.Services.prefs.getCharPref("extensions.firefox-muter.skin.updateurl");
    if (!url) {
      url = 'http://yxl.github.com/Firefox-Muter/update/skin.json' 
    }
    // Add a random number to the end fo the URL to ensure that we don't get the cached file.
    url += '?rnd=' + Math.random();
    this.httpGet(url, this.onWebDownloadFinished);
  },


  /**
   * Get a http file asynchronously.
   * @param url - The file URL
   * @param onreadystatechange - callback function after downloading.
   *
   * Callback function example:
   * httpGet("http://www.mozilla.com.cn", onHttpGetBack);
   * function onHttpGetBack(xmlHttpRequest) {
   *   if (4 == xmlHttpRequest.readyState && 200 == xmlHttpRequest.status) {
   *    let text = xmlHttpRequest.responseText;  // Text content  of the downloaded file
   *    alert(text);
   *   }
   * }
   */
  httpGet: function(url, onreadystatechange) {
    let xmlHttpRequest = new XMLHttpRequest();
    xmlHttpRequest.open('GET', url, true);
    xmlHttpRequest.send(null);
    xmlHttpRequest.onreadystatechange = function() {
      onreadystatechange(xmlHttpRequest);
      //if (4 == xmlHttpRequest.readyState && 200 == xmlHttpRequest.status) {
      //  onSuccess(xmlHttpRequest);
      //}
    };
  },

  onWebDownloadFinished: function(xmlHttpRequest) {
    if (4 == xmlHttpRequest.readyState && 200 == xmlHttpRequest.status) {
      try {
        let json = JSON.parse(xmlHttpRequest.responseText);
        if (!muterSkin.db.isSkinArrayEqual(json)) {
          muterSkin.db.skinArray = json;
          muterSkin.ui.rebuildSkinMenu();
        }
      } catch (ex) {
        Components.utils.reportError('onWebDownloadFinished:' + ex);
      }
    }
    muterSkin.ui._isSkinUpdating = false;
  }
};

/**
 *  Use preferences to store the skin data. 
 *　The skin data is a JSON object of the following format:
        [{
          name: {
            'en-US':'Super Mono',
            'zh-CN':'简洁立体',
            'zh-TW':'簡潔立體'
          },
          disabledIconUrl: 'chrome://muter/skin/icons/super-mono-disabled.png',
          enabledIconUrl: 'chrome://muter/skin/icons/super-mono-enabled.png',
          iconSource: 'http://www.lanrentuku.com/png/1326.html' 
        }, {
          name: {
            'en-US':'Four Color',
            'zh-CN':'简单四色',
            'zh-TW':'簡單四色'
          },
          disabledIconUrl: 'chrome://muter/skin/icons/simple-disabled.png',
          enabledIconUrl: 'chrome://muter/skin/icons/simple-enabled.png',
          iconSource: 'http://www.lanrentuku.com/png/14.html' 
        }]
 *  If the default name of the skin is empty, we use the name of current locale.
 *  The icon source URL tells where we get the original icons.
 *  
 *  @remarks - The preference can only store ASCII string. The non-ASCII string
 *  should be encoded to ASCII string before stored.
 * 
 */
muterSkin.db = {
  _prefName: "extensions.firefox-muter.skin.items",
  _items: null,

  init: function() {
    try {
      this._items = this._getJSONPref(this._prefName);
    } catch (ex) {}
    if (!this._items) {
      this._items = this._loadDefaultValue();
    }
  },

  get skinArray() {
    return this._items;
  },

  set skinArray(value) {
    this._items = value;
    this._setJSONPref(this._prefName, value);
  },

  isSkinArrayEqual: function(value) {
    try {
      return JSON.stringify(this._items) === JSON.stringify(value);
    } catch (ex) {
      return false;
    }
  },

  /**
   * Load the default skin data from the locale file of 
   *    chrome://muter/locale/defaultSkin.properties
   */
  _loadDefaultValue: function() {
    let MAX_NUM = 20;
    let defaultValue = [];
    let strings = muterUtils.Strings.defaultSkin;
    let locale = muterUtils.getLocaleString();
    for (let i = 1; i <= MAX_NUM; i++) {
      try {
        let name = strings.GetStringFromName("item" + i + ".name");
        let disabledIconUrl = strings.GetStringFromName("item" + i + ".disabledIconUrl");
        let enabledIconUrl = strings.GetStringFromName("item" + i + ".enabledIconUrl");
        if (name && disabledIconUrl && enabledIconUrl) {
          let item = {
            name: {},
            disabledIconUrl: disabledIconUrl,
            enabledIconUrl: enabledIconUrl
          };
          item.name[locale] = name;
          defaultValue.push(item);
        }
      } catch (ex) {
        break;
      }
    }
    return defaultValue;
  },

  /** 　
   * Load the JSON object from user preferences and decode the non-ASCII string with
   * decodeURIComponent.
   */
  _getJSONPref: function(name) {
    try {
      let jsonItems = decodeURIComponent(muterUtils.Services.prefs.getCharPref(name));
      let json = JSON.parse(jsonItems);
      return json;
    } catch (ex) {
      return null;
    }
  },

  /**
   *  Serialize the JSON object to string. Encode the string with encodeURIComponent
   *  and save to user preferences.
   */
  _setJSONPref: function(name, value) {
    muterUtils.Services.prefs.setCharPref(name, encodeURIComponent(JSON.stringify(value)));
  }
}

window.addEventListener("load", muterSkin.ui.load, false);
window.addEventListener("unload", muterSkin.ui.unload, false);