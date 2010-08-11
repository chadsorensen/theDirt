/* $Id: clusterer_marker.js,v 1.2 2008/07/15 16:30:30 bdragon Exp $ */

/**
 * GMap Markers
 * Jef Poskanzer's Clusterer.js API version
 */

// Replace to override marker creation
Drupal.gmap.factory.marker = function(loc,opts) {
  return new GMarker(loc,opts);
};

Drupal.gmap.addHandler('gmap', function(elem) {
  var obj = this;

  obj.bind('init',function() {
    obj.clusterer = new Clusterer(obj.map);
    var s = Drupal.settings.gmap_markermanager;
    if (s) {
      obj.clusterer.SetMaxVisibleMarkers(s.max_nocluster);
      obj.clusterer.SetMinMarkersPerCluster(s.cluster_min);
      obj.clusterer.SetMaxLinesPerInfoBox(s.max_lines);
    }
  });

  obj.bind('iconsready',function() {
    var s = Drupal.settings.gmap_markermanager;
    if (s) {
      obj.clusterer.SetIcon(Drupal.gmap.getIcon(s.marker,0));
    }
  });

  obj.bind('addmarker',function(marker) {
    var t = '';
    if (marker.opts.title) {
      t = marker.opts.title;
      if (marker.link) {
        t = '<a href="' + marker.link + '">' + t + '</a>';
      }
    }
    obj.clusterer.AddMarker(marker.marker,t);
  });

  obj.bind('delmarker',function(marker) {
    obj.clusterer.RemoveMarker(marker.marker);
  });

  obj.bind('clearmarkers',function() {
    // @@@ Maybe don't nuke ALL overlays?
    obj.map.clearOverlays();
  });
});
