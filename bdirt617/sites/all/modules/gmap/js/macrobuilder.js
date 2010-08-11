/* $Id: macrobuilder.js,v 1.2 2008/07/15 16:30:30 bdragon Exp $ */

////////////////////////////////////////
//           Map ID widget            //
////////////////////////////////////////
Drupal.gmap.addHandler('mapid', function(elem) {
  var obj = this;
  // Respond to incoming map id changes.
  var binding = obj.bind("idchange",function(){
    elem.value = obj.vars.macro_mapid;
  });
  // Send out outgoing map id changes.
  $(elem).change(function() {
    obj.vars.macro_mapid = elem.value;
    obj.change("idchange",binding);
  });
});
