// $Id: jquerymenu_no_animation.js,v 1.3 2009/11/16 08:44:48 aaronhawkins Exp $
Drupal.behaviors.jquerymenu = function(context) {
  $('ul.jquerymenu:not(.jquerymenu-processed)', context).addClass('jquerymenu-processed').each(function(){
    $(this).find("li.parent span.parent").click(function(){
      momma = $(this).parent();
      if ($(momma).hasClass('closed')){
        $(momma).removeClass('closed').addClass('open');
        $(this).removeClass('closed').addClass('open');
      }
      else{
        $(momma).removeClass('open').addClass('closed');
        $(this).removeClass('open').addClass('closed');
      }
    });
    showit = function() {
      $(this).children().show();
    }
    hideit = function() {
      $(this).children().hide();
    }
    $(this).find(".editbox").hover(showit, hideit);
  });
}