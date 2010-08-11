/* $Id: shapeloader_static.js,v 1.2 2008/07/15 16:30:30 bdragon Exp $ */

/**
 * GMap Shape Loader
 * Static Shapes.
 * This is a simple marker loader to read markers from the map settings array.
 * Commonly used with macros.
 */

// Add a gmap handler
Drupal.gmap.addHandler('gmap', function(elem) {
  var obj = this;
  var shape, i;
  if (obj.vars.shapes) {
    // Inject shapes during init.
    obj.bind('init',function() {
      // We need to move the incoming shapes out of the way,
      // because addshape will readd them, causing an infinate loop.
      // Store the shapes in s and reset obj.vars.shapes.
      var s = obj.vars.shapes;
      obj.vars.shapes = [];
      $.each(s, function(i,shape) {
        if (!shape.opts) {shape.opts = {};}
        // TODO: style props?
        // And add it.
        obj.change('prepareshape',-1,shape);
        obj.change('addshape',-1,shape);
      });
      obj.change('shapesready',-1);
    });
  }
});
