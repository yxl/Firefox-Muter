/*
 * This Source Code is subject to the terms of the Mozilla Public License
 * version 2.0 (the "License"). You can obtain a copy of the License at
 * http://mozilla.org/MPL/2.0/.
 *
 * The orginal codes of  feedback.js and feedback.xul are from the project
 * of adblock plus(https://adblockplus.org/). If you have installed adblock
 * plus extension on firefox. View related codes with the following URL:
 * chrome://adblockplus/content/ui/sendReport.js.
 */

var muterFeedback = (function() {
  let jsm = {};
  Components.utils.import("resource://muter/muterUtils.jsm", jsm);
  Components.utils.import("resource://gre/modules/Services.jsm", jsm);
  Components.utils.import("resource://gre/modules/AddonManager.jsm", jsm);
  Components.utils.import("resource://gre/modules/NetUtil.jsm", jsm);
  let {
    muterUtils, Services, AddonManager, NetUtil
  } = jsm;

  let prefs = Services.prefs;

  //
  // Report data template, more data will be added during data collection
  //
  let reportData = {
    type: 'other',
    comment: '',
    email: '',
    url: '',
    muter: {
        version: '1.0.0',
        locale: muterUtils.getLocaleString()
    },
    application: {
        name: Services.appinfo.name,
        vendor: Services.appinfo.vendor,
        version: Services.appinfo.version
    },
    platform: {
        name: muterUtils.platform.name,
        version: muterUtils.platform.version,
        is64: muterUtils.platform.is64
    },
    prefs: {
        'extensions.firefox-muter.firstRun': prefs.getBoolPref('extensions.firefox-muter.firstRun'),
        'extensions.firefox-muter.showInAddonBar': prefs.getBoolPref('extensions.firefox-muter.showInAddonBar'),
        'extensions.firefox-muter.switchButtonType': prefs.getCharPref('extensions.firefox-muter.switchButtonType'),
        'extensions.firefox-muter.saveStatus': prefs.getBoolPref('extensions.firefox-muter.saveStatus'),
        'extensions.firefox-muter.muteStatus': prefs.getBoolPref('extensions.firefox-muter.muteStatus')
    },
    addons: {
    }
  };

  // Get the extension's version
  AddonManager.getAddonByID("muter@yxl.name", function(addon) {
    reportData.muter.version = addon.version;
  });

  // Get report templates

  let reportTemplate = 'BODY_DATA';
  let reportPreviewTemplate = '"REPORT_DATA"';

  NetUtil.asyncFetch('chrome://muter/content/reportTemplate.html', function(inputStream, status) {
    if (!Components.isSuccessCode(status)) {
      return;
    }

    reportTemplate = NetUtil.readInputStreamToString(inputStream, inputStream.available());
  });

  NetUtil.asyncFetch('chrome://muter/content/reportPreviewTemplate.html', function(inputStream, status) {
    if (!Components.isSuccessCode(status)) {
      return;
    }

    reportPreviewTemplate = NetUtil.readInputStreamToString(inputStream, inputStream.available());
  });

  let muterFeedback = {

    initWizard: function(event) {
        let windowUrl = "";
        if (window.arguments && window.arguments.length > 0) {
            windowUrl = window.arguments[0];
        }
        document.getElementById('muter-feedback-url').value = windowUrl;

        addonsDataSource.collectData();

        // Change label of finish button to 'send'
        let wizard = document.getElementById('muter-feedback-wizard');
        if (wizard) {
            let finishButton = wizard.getButton('finish');
            if (finishButton) {
                finishButton.setAttribute("label", wizard.getAttribute("sendbuttonlabel"));
            }
        }
    },

    setProgresBarActiveItem: function(id) {
      let progressBar = document.getElementById('progressBar');
      if (progressBar) {
        progressBar.activeItem = document.getElementById(id);
      }
    }
  };

  muterFeedback.typeSelectorPage = {
    show: function(event) {
      muterFeedback.setProgresBarActiveItem('typeSelectorHeader');

      this.typeSelectionUpdated();
      return true;
    },

    typeSelectionUpdated: function() {
      let typeGroup = document.getElementById('muter-feedback-typeGroup');
      if (typeGroup && typeGroup.selectedItem) {
        reportData.type = typeGroup.selectedItem.value;
      }
    }

  };

  muterFeedback.commentPage = {
    show: function(event) {
      muterFeedback.setProgresBarActiveItem('commentPageHeader');

      this.updateExtensions();
      this.updateUrl();
      this.updateEmail();
      return true;
    },

    updateComment: function() {
      let value = document.getElementById('muter-feedback-comment').value;
      reportData.comment = value.substr(0, 500);
      let warningItem = document.getElementById('muter-feedback-commentLengthWarning');
      if (warningItem) {
        warningItem.hidden = value.length < 500;
      }
    },

    updateUrl: function() {
      let value = document.getElementById('muter-feedback-url').value;
      reportData.url = value;
    },

    updateEmail: function() {
      let value = document.getElementById('muter-feedback-email').value;
      reportData.email = value.replace(/\@/g, " at ").replace(/\./g, " dot ");
    },

    updateExtensions: function() {
      let checked = document.getElementById('muter-feedback-addonsCheckbox').checked;
      addonsDataSource.exportData(checked);
    },

  };

  muterFeedback.sendPage = {

    show: function() {
        muterFeedback.setProgresBarActiveItem('sendPageHeader');
        let dataField = document.getElementById('muter-feedback-report-data');
        if (dataField) {
          dataField.contentDocument.open();
          dataField.contentDocument.write(reportPreviewTemplate.replace('"REPORT_DATA"', JSON.stringify(reportData)));
          dataField.contentDocument.close();
        }
        let progress = document.getElementById('muter-feedback-sendReportProgress');
        if (progress) {
            progress.hidden = true;
        }
        let errorMsg = document.getElementById('muter-feedback-sendReportErrorBox');
        if (errorMsg) {
            errorMsg.hidden = true;
        }
        return true;
    },

    send: function() {
        let reportText = '';
        let dataField = document.getElementById('muter-feedback-report-data');
        if (dataField) {
            reportText = reportTemplate.replace('BODY_DATA', dataField.contentDocument.body.innerHTML);
        } else {
            reportText = JSON.stringify(reportData);
        }
        let progress = document.getElementById('muter-feedback-sendReportProgress');
        if (progress) {
            progress.hidden = false;
        }
        let errorMsg = document.getElementById('muter-feedback-sendReportErrorBox');
        if (errorMsg) {
            errorMsg.hidden = true;
        }

        let request = new XMLHttpRequest();
              let url = prefs.getCharPref("extensions.firefox-muter.feedbackUrl");
        request.open("POST", url);
        request.setRequestHeader("Content-Type", "text/html");
        request.addEventListener("load", muterFeedback.sendPage._reportSent, false);
        request.addEventListener("error", muterFeedback.sendPage._reportSent, false);
        request.send(reportText);

        return false;
    },

    _reportSent: function(event) {
        let request = event.target;
        let success = false;
        try {
          let status = request.channel.status;
          if (Components.isSuccessCode(status)){
            success = (request.status == 200 || request.status == 0);
          }
        } catch (e) {}

        // Close the wizard if the report is sent successfully.
        if (success) {
            window.close();
        }

        let progress = document.getElementById('muter-feedback-sendReportProgress');
        if (progress) {
            progress.hidden = true;
        }
        let errorMsg = document.getElementById('muter-feedback-sendReportErrorBox');
        if (errorMsg) {
            errorMsg.hidden = false;
        }

        // Change label of finish button to 'retry'
        let wizard = document.getElementById('muter-feedback-wizard');
        if (wizard) {
            let finishButton = wizard.getButton('finish');
            if (finishButton) {
                finishButton.setAttribute("label", wizard.getAttribute("retrybuttonlabel"));
            }
        }
    }
  };

  let addonsDataSource =  {
    _data: {},

    collectData: function() {
      // Gecko 2.0, Firfox 4+
      let self = this;
      AddonManager.getAddonsByTypes(["extension", "plugin"], function(items){
        for (let i = 0; i < items.length; i++){
          let item = items[i];
          if (!item.isActive)
            continue;
          self._data[item.id] = {name: item.name, type: item.type, version: item.version};
        }
      });
    },

    exportData: function(doExport) {
      if (doExport)
        reportData.addons = this._data;
      else
        delete reportData.addons;
    }
  };

  return muterFeedback;
})();