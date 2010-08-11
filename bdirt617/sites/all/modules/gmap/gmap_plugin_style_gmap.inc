<?php
// $Id: gmap_plugin_style_gmap.inc,v 1.3 2008/11/24 22:47:12 bdragon Exp $

/**
 * @file
 * GMap style plugin.
 */

/**
 * Style plugin to render a map.
 *
 * @ingroup views_style_plugins
 */
class gmap_plugin_style_gmap extends views_plugin_style {
  /**
   * Set default options
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['macro'] = array(
      'default' => '[gmap ]',
    );

    $options['datasource'] = array(
      'default' => 'location',
    );

    $options['markers'] = array('default' => 'static');
    $options['markertype'] = array('default' => 'drupal');

    return $options;
  }

  function query() {
    if ($this->options['datasource'] == 'location') {
      $this->view->query->add_field('location', 'latitude', 'gmap_lat');
      $this->view->query->add_field('location', 'longitude', 'gmap_lon');
    }

    if ($this->options['markers'] == 'nodetype') {
      $this->view->query->add_field('node', 'type', 'gmap_node_type');
    }
    else if ($this->options['markers'] == 'taxonomy') {
      $this->view->query->add_field('gmap_taxonomy_node', 'marker', 'gmap_node_marker');
    }
  }

  function render() {
    if (empty($this->row_plugin)) {
      vpr('gmap_plugin_style_gmap: Missing row plugin');
      return;
    }

    $markername = isset($this->options['markertype']) ? $this->options['markertype'] : 'drupal';

    $markertypes = variable_get('gmap_node_markers', array());
    if ($this->options['markers'] == 'nodetype') {
      $markertypes = variable_get('gmap_node_markers', array());
    }

    // Group the rows according to the grouping field, if specified.
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);

    // Render each group separately and concatenate.  Plugins may override this
    // method if they wish some other way of handling grouping.
    $output = '';
    foreach ($sets as $title => $records) {
      $markers = array();
      $offsets = array();
        foreach ($records as $label => $row) {
        $lat = (float)$row->gmap_lat;
        $lon = (float)$row->gmap_lon;
        if (!empty($lat) && !empty($lon)) {
          if ($this->options['markers'] == 'nodetype') {
            if (isset($markertypes[$row->gmap_node_type])) {
              $markername = $markertypes[$row->gmap_node_type];
            }
          }
          else if ($this->options['markers'] == 'taxonomy') {
            if (!empty($row->gmap_node_marker)) {
              $markername = $row->gmap_node_marker;
            }
          }
          if (!isset($offsets[$markername])) {
            $offsets[$markername] = 0;
          }
          $markers[] = array(
            'latitude' => $lat,
            'longitude' => $lon,
            'markername' => $markername,
            'offset' => $offsets[$markername],
            'text' => $this->row_plugin->render($row),
          );
          $offsets[$markername]++;
        }
      }
      if (!empty($markers)) { // Don't draw empty maps.
        $map = gmap_parse_macro($this->options['macro']);
        $map['markers'] = $markers;
        $map['id'] = gmap_get_auto_mapid();
        $output .= theme($this->theme_functions(), $this->view, $this->options, $map, $title);
      }
    }
    return $output;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['macro'] = array(
      '#type' => 'textfield',
      '#title' => t('Macro'),
      '#size' => 1000,
      '#default_value' => $this->options['macro'],
    );

    $form['datasource'] = array(
      '#type' => 'radios',
      '#title' => t('Data Source'),
      '#options' => array(
        'location' => t('Location.module'),
        'autodetect' => t('Fields named "latitude" and "longitude" @@@TODO'),
      //'geocode' => t('Just-in-time geocoding on field named "address"'),
      ),
      '#default_value' => $this->options['datasource'],
      '#multiple' => FALSE,
    );

    $form['markers'] = array(
      '#type' => 'radios',
      '#title' => t('Marker handling'),
      // @@@ Detect view type automatically?
      '#options' => array(
        'nodetype' => t('By content type (for node views)'),
        'taxonomy' => t('By term (for node views)'),
        'static' => t('Use single marker type'),
      ),
      '#default_value' => $this->options['markers'],
    );

    // Hide the taxonomy handling if gmap_taxonomy.module isn't installed.
    if (!module_exists('gmap_taxonomy')) {
      unset($form['markers']['#options']['taxonomy']);
    }

    $form['markertype'] = array(
      '#type' => 'gmap_markerchooser',
      '#title' => t('Marker / fallback marker to use'),
      '#default_value' => $this->options['markertype'],
    );
  }
}
