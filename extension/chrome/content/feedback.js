/*
 * This Source Code is subject to the terms of the Mozilla Public License
 * version 2.0 (the "License"). You can obtain a copy of the License at
 * http://mozilla.org/MPL/2.0/.
 */

var muterFeedback = (function() {
  let jsm = {};
  Components.utils.import("resource://muter/muterUtils.jsm", jsm);
  let {
    muterUtils
  } = jsm;
  
  let prefs = muterUtils.Services.prefs;

  //
  // Report data template, more data will be added during data collection
  //
  let reportData =
  <report>
                <comment />
                <email />
                <url />
    <muter version="" locale={muterUtils.getLocaleString()}/>
    <application name={muterUtils.Services.appinfo.name} vendor={muterUtils.Services.appinfo.vendor} version={muterUtils.Services.appinfo.version} userAgent={window.navigator.userAgent}/>
    <platform name={muterUtils.platform.name} version={muterUtils.platform.version} is64={muterUtils.platform.is64}/>
    <prefs>
                    <pref key="extensions.firefox-muter.firstRun" value={prefs.getBoolPref("extensions.firefox-muter.firstRun")} />
                    <pref key="extensions.firefox-muter.showInAddonBar" value={prefs.getBoolPref("extensions.firefox-muter.showInAddonBar")} />
                    <pref key="extensions.firefox-muter.showInStatusBar" value={prefs.getBoolPref("extensions.firefox-muter.showInStatusBar")} />
                    <pref key="extensions.firefox-muter.skin.items" value={prefs.getCharPref("extensions.firefox-muter.skin.items")} />
                    <pref key="extensions.firefox-muter.switchButtonType" value={prefs.getCharPref("extensions.firefox-muter.switchButtonType")} />
                    <pref key="dom.ipc.plugins.enabled" value={prefs.getBoolPref("dom.ipc.plugins.enabled")} />
                </prefs>
    <addons />
  </report>;

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
        reportData.@type = typeGroup.selectedItem.value;
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
          let [selectionStart, selectionEnd] = [dataField.selectionStart, dataField.selectionEnd];
          dataField.value = reportData.toXMLString();
          dataField.setSelectionRange(selectionStart, selectionEnd);
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
        request.setRequestHeader("Content-Type", "text/xml");
        request.addEventListener("load", muterFeedback.sendPage._reportSent, false);
        request.addEventListener("error", muterFeedback.sendPage._reportSent, false);
        request.send(reportData.toXMLString());
             
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
    _data: <addons />,

    collectData: function() {
      let AddonManager = null;
      try{
        let namespace = {};
        Components.utils.import("resource://gre/modules/AddonManager.jsm", namespace);
        AddonManager = namespace.AddonManager;
      } catch (e) {}

      if (AddonManager) {
        // Gecko 2.0, Firfox 4+
        let me = this;
        AddonManager.getAddonsByTypes(["extension", "plugin"], function(items){
          for (let i = 0; i < items.length; i++){
            let item = items[i];
            if (!item.isActive)
              continue;
            me._data.appendChild(<addon id={item.id} name={item.name} type={item.type} version={item.version}/>);
          }
        });
      }
      else if ("@mozilla.org/extensions/manager;1" in Components.classes) {
        // Gecko 1.9.x, Firefox 3
        let extensionManager = Components.classes["@mozilla.org/extensions/manager;1"].getService(Components.interfaces.nsIExtensionManager);
        let ds = extensionManager.datasource;
        let rdfService = Components.classes["@mozilla.org/rdf/rdf-service;1"].getService(Components.interfaces.nsIRDFService);
        let list = {};
        let items = extensionManager.getItemList(Components.interfaces.nsIUpdateItem.TYPE_EXTENSION, {});
        for (let i = 0; i < items.length; i++) {
          let item = items[i];

          // Check whether extension is disabled - yuk...
          let source = rdfService.GetResource("urn:mozilla:item:" + item.id);
          let link = rdfService.GetResource("http://www.mozilla.org/2004/em-rdf#isDisabled");
          let target = ds.GetTarget(source, link, true);
          if (target instanceof Components.interfaces.nsIRDFLiteral && target.Value == "true")
            continue;

          this._data.appendChild(<addon id={item.id} name={item.name} type="extension" version={item.version}/>);
        }
        
        function isPluginEnabled(plugin) {
            for (let i=0; i<plugin.length; i++) {
                if (plugin.item(i).enabledPlugin) {
                    return true;
                }
            }
            return false;
        }
        let plugins = navigator.plugins;
        for (let i = 0; i < plugins.length; i++) {
            let plugin = plugins.item(i);
            if (!isPluginEnabled(plugin)) {
                continue;
            }
            this._data.appendChild(<addon id={plugin.filename} name={plugin.name} type="plugin" version={plugin.version}/>);
        }
        
      }
      else {
        // No add-on manager, no extension manager - we must be running in K-Meleon.
        // Skip this step.
      }
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