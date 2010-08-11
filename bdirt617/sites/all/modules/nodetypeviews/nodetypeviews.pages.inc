<?php
// $Id: nodetypeviews.pages.inc,v 1.1 2008/08/30 22:08:45 yaph Exp $

/**
 * @file
 * Page callback file for the Node Type Views module.
 */

/**
 * Create the teaser list for the given node type
 *
 * @param String $type
 * @return String $output - the HTML markup of the view
 */
function nodetypeviews_type_teaser_view($type) {
  $output = '';
  $result = pager_query("SELECT n.nid, n.created FROM {node} n WHERE n.type = '%s' AND n.status = 1 ORDER BY n.sticky DESC, n.created DESC", variable_get('default_nodes_main', 10), 0, NULL, $type);
  while ($node = db_fetch_object($result)) {
    $output .= node_view(node_load($node->nid), 1);
  }
  $output .= theme('pager', NULL, variable_get('default_nodes_main', 10));
  drupal_add_feed(url($type .'/feed'), t('RSS - '. $type));
  return $output;
}

/**
 * Create an RSS feed for the given node type
 *
 * @param String $type
 * @return String - the XML feed
 */
function nodetypeviews_type_teaser_feed($type) {
  $result = db_query_range(db_rewrite_sql("SELECT n.nid, n.created FROM {node} n WHERE n.type = '%s' AND n.status = 1 ORDER BY n.created DESC"), $type, 0, variable_get('feed_default_items', 10));
  $channel['title'] = variable_get('site_name', 'Drupal') .' '. $type;
  $channel['link'] = url($type, array('absolute' => TRUE));
  $channel['description'] = $term->description;
  node_feed($result, $channel);
}