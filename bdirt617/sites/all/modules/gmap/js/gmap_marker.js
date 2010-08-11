/* $Id: gmap_marker.js,v 1.2 2008/07/15 16:30:30 bdragon Exp $ */

/**
 * GMap Markers
 * GMap API version -- No manager
 */

// Replace to override marker creation
Drupal.gmap.factory.marker = function(loc,opts) {
  return new GMarker(loc,opts);
};

Drupal.gmap.addHandler('gmap', function(elem) {
  var obj = this;

  obj.bind('addmarker',function(marker) {
    obj.map.addOverlay(marker.marker);
  });

  obj.bind('delmarker',function(marker) {
    obj.map.removeOverlay(marker.marker);
  });

  obj.bind('clearmarkers',function() {
    // @@@ Maybe don't nuke ALL overlays?
    obj.map.clearOverlays();
  });
});
