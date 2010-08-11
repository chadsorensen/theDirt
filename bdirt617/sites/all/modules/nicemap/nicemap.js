if (typeof(Drupal) == "undefined" || !Drupal.nicemap) {
  Drupal.nicemap = {};
}

Drupal.nicemap.iehover = function() {
  // Provide consistent hovering
  $('div.nicemap-map a.geopoint').hover(function(){
    $(this).addClass('hover');
  }, function() {
    $(this).removeClass('hover');
  });
}

if (Drupal.jsEnabled) {
  $(document).ready(function() {
    if ($.browser.msie) {
      Drupal.nicemap.iehover();
    }
  });
};
