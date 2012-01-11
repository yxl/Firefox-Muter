/*
 * This Source Code is subject to the terms of the Mozilla Public License
 * version 2.0 (the "License"). You can obtain a copy of the License at
 * http://mozilla.org/MPL/2.0/.
 */

var muterFeedback = (function(){
  let jsm = {};
  Components.utils.import("resource://muter/muterUtils.jsm", jsm);
  let { muterUtils } = jsm;
  
  let muterFeedback = {
  
    initWizard: function(event) {
    },    
  };

  return muterFeedback;
})();