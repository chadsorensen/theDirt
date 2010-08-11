<?php
// $Id: nicemap_views.theme.inc,v 1.1 2008/11/14 18:03:58 jmiccolis Exp $

/**
 * Theme preporocess for nicemap view style.
 */
function template_preprocess_views_view_nicemap_map(&$vars) {
  $view = $vars['view'];
  $options = $view->style_plugin->options;

  $map = $view->style_plugin->build_map();
  $points = $view->style_plugin->map_points($vars['rows']);

  $vars['map'] = theme('nicemap_map', $points, $map, $options['width'], $options['height'], true);
}
