<?php
// $Id: nodetypeviews.admin.inc,v 1.2 2008/08/30 22:54:39 yaph Exp $

/**
 * @file
 * Admin settings for the Node Type Views module.
 */

/**
 * Admin settings form
 */
function nodetypeviews_admin_settings() {
  $form = array();
  $form['nodetypeviews_node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Content Types'),
    '#default_value' => variable_get('nodetypeviews_node_types', array()),
    '#options' => node_get_types('names'),
    '#description' => t("Select the content types to generate views for. Do not enable Blog entry, unless it is a content type you created the blog module takes care of that.")
  );
  return system_settings_form($form);
}