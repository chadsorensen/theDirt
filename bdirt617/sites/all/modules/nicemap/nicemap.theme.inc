<?php
// $Id: nicemap.theme.inc,v 1.1 2008/11/14 18:03:58 jmiccolis Exp $

/**
 * Wrapper theme function allows for easy overriding of map style
 *
 * @param $points
 *   Array An array of points in the form:
 *       array('title' => 'Chester',
 *           'content' => 'Test',
 *           'lat' => 40.47,
 *           'lon' => -74.41,
 *           'weight' => 5,
 *           ),
 *  @param $map
 *   nicemap_map object
 *  @param $width
 *   int Desired width of the resulting image
 *  @param $height 
 *   int Desire height of the resulting image
 *
 *  @return
 *   A fully themed map
 */
function theme_nicemap_map($points = array(), $map, $width = 200, $height = 200, $js = true) {
  drupal_add_css(drupal_get_path('module', 'nicemap') .'/nicemap.css');
  if ($js) {
    drupal_add_js(drupal_get_path('module', 'nicemap') .'/nicemap.js');
  }
  $points = $map->process($points, array('width' => $width, 'height' => $height));
  $options = array(
    'width' => $width,
    'height' => $height,
  );
  return _theme_nicemap_map_full($points, $map, $options);
}

/**
 * Theme a point on a map
 *
 * @param $point 
 *   an array which, by default requires the keys
 *   title, weight, x, y and an optional array of attributes
 * 
 */
function theme_nicemap_point($point) {
  $weight = $point['weight'] ? ' weight-'. $point['weight'] : '';
  $title = "<span>". strip_tags($point['title']) ."</span>";
  $point['attributes']['class'] = $point['attributes']['class'] ?
    $point['attributes']['class'] ." geopoint $weight" :
    "geopoint $weight";

  // sorry, we're gonna bulldoze your style attributes
  $point['attributes']['style'] = 'left: '. $point['x'] .'%; top: '. $point['y'] .'%;';
  // and your id attributes
  $point['attributes']['id'] = "geopoint-". $point['i'];
  return l($title, $point['href'], array('attributes' => $point['attributes'], 'query' => null, 'fragment' => null, 'absolute' => TRUE, 'html' => TRUE));
}

function theme_nicemap_content($point) {
  $content = $point['content'];
  $close = "<span class='close'>". t('Close') ."</span>";
  return "<div class='geoitem' id='geoitem-". $point['i'] ."'>$close $content</div>";
}

function _theme_nicemap_map_full($points, $map, $options) {
  $w = $options['width']  .'px';
  $h = $options['height'] .'px';
  $map_url = $map->url($options);
  if ($map_url) {
    $map_bg = "background-image:url('$map_url');";
  }

  // use a hardcoded index to associate point to item
  $i = 0;
  foreach ($points as $point) {
    $point['i'] = $i;
    $geopoints  .= theme('nicemap_point', $point);
    $geocontent .= theme('nicemap_content', $point);
    $i++;
  }

  $attr = array(
    "class" => "nicemap-map",
    "style" => "$map_bg width:$w; height:$h;",
  );

  $attr = drupal_attributes($attr);

  return "
    <div $attr>
      $geopoints
      <div class='hidden'>$geocontent</div>
    </div>
  ";
}