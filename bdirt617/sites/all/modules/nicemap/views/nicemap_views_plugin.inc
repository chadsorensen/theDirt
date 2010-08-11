<?php
// $Id: nicemap_views_plugin.inc,v 1.2.2.2 2008/12/12 14:47:26 jmiccolis Exp $

/**
 * Extending the view_plugin_style class to provide a nicemap view style.
 */
class nicemap_views_plugin extends views_plugin_style {
  /**
   * Set default options
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['width'] = array('default' => 200);
    $options['height'] = array('default' => 200);
    $options['minx'] = array('default' => -100);
    $options['miny'] = array('default' => -40);
    $options['maxx'] = array('default' => 100);
    $options['maxy'] = array('default' => 40);
    $options['custom'] = array('default' => false);
    $options['layers'] = array('default' => array());
    $options['styles'] = array('default' => array());
    $options['weights'] = array('default' => array());
    $options['bgcolor'] = array('default' => 'aabbcc');
    $options['fields'] = array('default' => array());

    return $options;
  }

  function options_form(&$form, &$form_state) {    
    $form['width'] = array(
      '#type' => 'textfield',
      '#title' => t('Width'),
      '#default_value' => $this->options['width'],
      '#size' => 25,
      '#maxlength' => 6,
      '#description' => t('The width of the map (in units of px), e.g. 600. Leave blank to use default value.'),
    );
    $form['height'] = array(
      '#type' => 'textfield',
      '#title' => t('Height'),
      '#default_value' => $this->options['height'],
      '#size' => 25,
      '#maxlength' => 6,
      '#description' => t('The height of the map (px), e.g. 400. Leave blank to use default value.'),
    );
    $form['bounds'] = array(
      '#type' => 'fieldset',
      '#title' => t('Bounding Box'),
      '#description' => t('In decimal form, the min and max longitude and latitude of the map'),
      '#collapsible' => true,
      '#collapsed' => true,
    );
    $form['bounds']['minx'] = array(
      '#type' => 'textfield',
      '#title' => t('Min Longitude'),
      '#default_value' => $this->options['minx'],
      '#size' => 25,
    );
    $form['bounds']['miny'] = array(
      '#type' => 'textfield',
      '#title' => t('Min Latitude'),
      '#default_value' => $this->options['miny'],
      '#size' => 25,
    );
    $form['bounds']['maxx'] = array(
      '#type' => 'textfield',
      '#title' => t('Max Longitude'),
      '#default_value' => $this->options['maxx'],
      '#size' => 25,
    );
    $form['bounds']['maxy'] = array(
      '#type' => 'textfield',
      '#title' => t('Max Latitude'),
      '#default_value' => $this->options['maxy'],
      '#size' => 25,
    );

    if ($wms = variable_get('nicemap_wms_url', '')) {
      module_load_include('inc', 'nicemap', 'nicemap_admin');
      $spec = nicemap_capabilities_cache($wms);
      $l = _nicemap_get_layers($spec);
      _nicemap_layer_style_form($form, $l, $this->options['layers'], $this->options['styles'], $this->options['weights'], $this->options['nicemap_bgcolor']);
      $form['nicemap_defaults']['#collapsible'] = true;
      $form['nicemap_defaults']['#collapsed'] = !$this->options['custom'];
      $form['display']['custom'] = array(
        '#type' => 'checkbox',
        '#title' => t('Customize'),
        '#default_value' => $this->options['custom'],
        '#description' => t('If checked view specific "layer display options", defined below, will be used.'),
      );
    }
    else {
      $form['error_wms'] = array(
        '#value' => t('You must set a WMS url. !link', array('!link' => l(t('link'), 'admin/settings/nicemap'))),
        '#prefix' => '<div class="error form-item description">',
        '#suffix' => '</div>',
      );
    }
    
    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      $form['error_markup'] = array(
        '#value' => t('You need at least one field before you can configure your field settings'),
        '#prefix' => '<div class="error form-item description">',
        '#suffix' => '</div>',
      );
    }
    else {
      $field_names[$field] = array('' => '--');
      foreach ($handlers as $field => $handler) {
        if ($label = $handler->label()) {
          $field_names[$field] = $label;
        }
        else {
          $field_names[$field] = $handler->ui_name();
        }
      }
      $field_options = array(
        'title' => t('Title'),
        'latitude' => t('Latitude'),
        'longitude' => t('Longitude'),
        'class' => t('Class'),
      );
      $form['fields'] = array(
        '#type' => 'fieldset',
        '#title' => 'Field usage',
        '#description' => t('Select the fields that contain the latitude,
                            longitude and title of each point. If selected, the
                            class field will be used to apply a class to each 
                            point. Remaining fields will be available in the 
                            hidden "content" region of the point.'),
      );
      foreach ($field_options as $k => $v) {
        $form['fields'][$k] = array(
          '#type' => 'select',
          '#title' => $v,
          '#options' => $field_names,
          '#default_value' => $this->options['fields'][$k],
          '#required' => ($k == 'class' ? false : true),
        );
      }
    }
    
    parent::options_form($form, $form_state);
  }

  function options_submit(&$form, &$form_state) {
    // Flatten variables
    foreach ($form_state['values']['style_options']['bounds'] as $key => $value) {
      $form_state['values']['style_options'][$key] = $value;
    }
    unset($form_state['values']['style_options']['bounds']);

    foreach ($form_state['values']['style_options']['display'] as $key => $value) {
      $form_state['values']['style_options'][$key] = $value;
    }
    unset($form_state['values']['style_options']['display']);

    foreach ($form_state['values']['style_options']['nicemap_defaults'] as $key => $value) {
      $form_state['values']['style_options'][$key] = $value;
    }
    unset($form['nicemap_defaults']);
  }

  /**
   * Generate map array.
   */
  function build_map() {    
    $map = new nicemap_map(false);
    $map->bgcolor = $this->options['bgcolor'];
    $map->bounds = array(
      'miny' => $this->options['miny'],
      'maxy' => $this->options['maxy'],
      'minx' => $this->options['minx'],
      'maxx' => $this->options['maxx'],
    );

    if ($this->options['custom']) {      
      $layers = $this->options['layers'];
      $weights = $this->options['weights'];
      $styles = $this->options['styles'];
    }
    else {
      $settings = variable_get('nicemap_defaults', array('layers' => array(), 'styles' => array(), 'weights' => array()));
      $layers = $settings['layers'];
      $weights = $settings['weights'];
      $styles = $settings['styles'];
    }

    $visible_layers = array_filter($layers);
    asort($this->options['weights']);

    foreach ($weights as $k => $v) {
      if (isset($visible_layers[$k])) {
        $map->layers[] = $k;
        $map->styles[] = $styles[$k];
      }
    }

    return $map;
  }

  /**
   * Process points.
   *
   * @param $rows
   *   Rows array, result of the views query
   *
   * @return
   *   Array of points suitable to be passed to theme_nicemap_map().
   */
  function map_points($rows) {
    $points = array();
    foreach ($rows as $id => $row) {
      $point = array('href' => 'node/'. $row->nid);
      foreach ($this->view->field as $key => $field) {
        if ($key == $this->options['fields']['title']) {
          $point['title'] = $field->theme($row);
        }
        elseif ($key == $this->options['fields']['latitude']) {
          $point['lat'] = $field->theme($row);
        }
        elseif ($key == $this->options['fields']['longitude']) {
          $point['lon'] = $field->theme($row);
        }
        elseif ($key == $this->options['fields']['class']) {
          $point['attributes']['class'] = $this->map_point_class($field->theme($row));
        }
        else {
          $point['content'] .= $field->theme($row);
        }
      }
      $points[] = $point;
    }
    return $points;
  }
  
  function map_point_class($str) {
    return strtolower(strtr(strip_tags($str), ' _', '--'));
  }
}

