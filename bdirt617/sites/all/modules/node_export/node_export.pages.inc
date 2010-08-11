<?php
// $Id: node_export.pages.inc,v 1.1.2.7 2009/06/25 11:52:20 danielb Exp $

/**
 * Menu callback to configure module settings.
 */
function node_export_settings() {

  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
  );
  $form['basic']['node_export_method'] = array(
    '#type' => 'radios',
    '#title' => t('Method to use when importing a node'),
    '#options' => array('prepopulate' => t('Pre-populate the node form fields'), 'save-edit' => t('Save as a new node then edit')),
    '#default_value' => variable_get('node_export_method', 'prepopulate'),
  );

  $form['publishing'] = array(
    '#type' => 'fieldset',
    '#title' => t('Reset values on import'),
  );
  $types = node_get_types('names');

  foreach ($types as $type => $name) {
    $form['publishing'][$type] = array(
      '#type' => 'fieldset',
      '#title' => $name,
      '#description' => t('Reset these values when importing nodes of type @s.', array('@s' => $name)),
    );
    $form['publishing'][$type]['node_export_reset_'. $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Publishing options (status, moderate, promote, sticky, and revision)'),
      '#default_value' => variable_get('node_export_reset_'. $type, FALSE),
    );
    $form['publishing'][$type]['node_export_reset_created_'. $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Created time (<em>Authored on</em> date)'),
      '#default_value' => variable_get('node_export_reset_created_'. $type, TRUE),
    );
    $form['publishing'][$type]['node_export_reset_menu_'. $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Menu link'),
      '#default_value' => variable_get('node_export_reset_menu_'. $type, TRUE),
    );
    $form['publishing'][$type]['node_export_reset_path_'. $type] = array(
      '#type' => 'checkbox',
      '#title' => t('URL path'),
      '#default_value' => variable_get('node_export_reset_path_'. $type, TRUE),
    );
    $form['publishing'][$type]['node_export_reset_book_mlid_'. $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Book menu link'),
      '#default_value' => variable_get('node_export_reset_book_mlid_'. $type, TRUE),
    );
  }
  // Need the variable default key to be something that's never a valid node type.
  $form['omit'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content types that are not to be exported - omitted due to incompatibility'),
  );
  $form['omit']['node_export_omitted'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Omitted content types'),
    '#default_value' => variable_get('node_export_omitted', array()),
    '#options' => $types,
    '#description' => t('Select any node types which should <em>never</em> be exported. Typically you should will want to select here all node types for which importing fails (e.g. image nodes).'),
  );

  return system_settings_form($form);
}

/**
 *  exports a node - populate a node code form
 *  set $return_code to TRUE to not return form but the code instead.
 */
function node_export_node_export($original_node, $return_code = FALSE, $iteration = 0) {
  if (isset($original_node->nid)) {
    global $user;

    if (node_export_is_permitted($original_node->type)) {
      $node = drupal_clone($original_node);

      drupal_set_title(t('Export of !title', array('!title' => check_plain($node->title))));

      // Fix taxonomy array
      if (isset($node->taxonomy) && count($node->taxonomy)) {
        $vocabularies = taxonomy_get_vocabularies();
        $new_taxonomy = array();
        foreach ($node->taxonomy as $term) {
          // Free-tagging vocabularies need a different format
          if ($vocabularies[$term->vid]->tags) {
            $new_taxonomy['tags'][$term->vid][] = $term->name;
          }
          else {
            $new_taxonomy[$term->vid][$term->tid] = $term->tid;
          }
        }
        if (isset($new_taxonomy['tags']) && count($new_taxonomy['tags'])) {
          // Comma seperate the tags
          foreach ($new_taxonomy['tags'] as $vid => $tags) {
            $new_taxonomy['tags'][$vid] = implode(', ', $tags);
          }
        }
        $node->taxonomy = $new_taxonomy;
      }

      // Fix menu array
      $node->menu = node_export_get_menu($original_node);

      // Let other modules do special fixing up.
      // The function signature is: hook_export_node_alter(&$node, $original_node, $method)
      // Where $method is 'export'
      drupal_alter("node_export_node", $node, $original_node, "export");
      $node_code = node_export_node_encode($node, $iteration);
      return ($return_code ? $node_code : drupal_get_form('node_export_form', $node_code));
    }
  }
}

/**
 *  Export Form
 */
function node_export_form($form_state, $code) {
  $form = array();
  $form['export'] = array(
    '#type' => 'textarea',
    '#title' => t('Node Code'),
    '#default_value' => $code,
    '#rows' => 30,
    '#description' => t('Copy this code and then on the site you want to import to, go to the Import Node link under Content Management, and paste it in there.'),
    '#attributes' => array(
      'style' => 'width: 97%;',
    ),
  );
  return $form;
}

/**
 *  Import Node UI
 */
function node_export_node_import() {
  global $user;
  $output;
  if ($_POST['import_code']) {
    $node_code = trim($_POST['import_code']);
    $import = node_export_node_decode($node_code);
    $types_exist = node_export_import_types_check($import);
    if ($types_exist) {
      if (is_array($import)) {
        foreach ($import as $new_node) {
          $new_nid = node_export_node_save($new_node);
          drupal_set_message(t("Imported !node", array('!node' => l($new_node->title, 'node/'. $new_nid))));
        }
        drupal_goto('admin/content/node');
      }
      else {
        return node_export_node_check($import);
      }
    }
  }
  return drupal_get_form('node_export_import_form', $form);
}

/**
 *  Check if all types in the import exist.  Return TRUE/FALSE.
 */
function node_export_import_types_check($import) {
  $types_exist = TRUE;
  if (is_array($import)) {
    foreach($import as $new_node) {
      if (node_get_types('name', $new_node) == FALSE) {
        $types_exist = $new_node->type;
      }
    }
  }
  else {
    $new_node = $import;
    if (node_get_types('name', $new_node) == FALSE) {
      $types_exist = $new_node->type;
    }
  }

  if ($types_exist !== TRUE) {
    drupal_set_message(t('Error encountered during import.  Node type %t unknown on this site.  No nodes imported.', array('%t' => $types_exist)), 'error');
    $types_exist = FALSE;
  }

  return $types_exist;
}

/**
 *  Import Form
 */
function node_export_import_form($form_state) {
  $form['#prefix'] = t('<p>You may import a node by copy-and-pasting the results of an exported node. </p>');
  $method = variable_get('node_export_method', 'prepopulate');
  if ($method == 'prepopulate') {
    $form['#prefix'] .= t('<p>You will be given a prepopulated node add form which you must save to create the node.</p>');
  }
  elseif ($method == 'save-edit') {
    $form['#prefix'] .= t('<p>The node will be created and you will then be taken to the node edit page.</p>');
  }
  $form['#prefix'] .= t('<p>To change this behaviour, <a href="/admin/settings/node_export">configure the settings</a>.</p>');
  $form['import_code'] = array(
    '#type' => 'textarea',
    '#title' => t('Import Code'),
    '#default_value' => '',
    '#required' => TRUE,
    '#rows' => 30,
    '#description' => t('Cut and paste the results of an <em>Export Node</em> here.'),
  );
  $form['#redirect'] = FALSE;
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );
  return $form;
}

/**
 *  Menu callback: prompt the user to confirm the operation
 */
function node_export_node_check($node) {

  $method = variable_get('node_export_method', 'prepopulate');

  switch ($method) {
    case 'save-edit':
      $new_nid = node_export_node_save($node);
      drupal_goto('node/'. $new_nid .'/edit');
      break;
    case 'prepopulate':
    default:
      include_once(drupal_get_path('module', 'node') .'/node.pages.inc');
      return node_export_node_prepopulate($node);
      break;
  }
}

/**
 *  exports a node - prepopulate a node editing form
 */
function node_export_node_prepopulate($original_node) {
  if (node_export_is_permitted($original_node->type)) {
    $node = node_export_node_clone($original_node, "prepopulate");
    return str_replace('admin/content/import', 'node/add/' . $node->type, drupal_get_form($node->type . '_node_form', $node));
  }
}

/**
 *  exports a node by directly saving it.
 */
function node_export_node_save($original_node) {
  if (node_export_is_permitted($original_node->type)) {
    $node = node_export_node_clone($original_node, "save-edit");
    node_save($node);
    return $node->nid;
  }
}

/**
 *  Prepare a clone of the node during import.
 */
function node_export_node_clone($original_node, $mode) {
    global $user;

    $node = drupal_clone($original_node);

    $node->nid = NULL;
    $node->vid = NULL;

    $node->name = $user->name;
    $node->uid = $user->uid;

    if (variable_get('node_export_reset_created_'. $node->type, TRUE)) {
      $node->created = NULL;
    }

    if (variable_get('node_export_reset_menu_'. $node->type, TRUE)) {
      $node->menu = NULL;
    }

    if (variable_get('node_export_reset_path_'. $node->type, TRUE)) {
      $node->path = NULL;
    }

    if (variable_get('node_export_reset_book_mlid_'. $node->type, TRUE) && isset($node->book['mlid'])) {
      $node->book['mlid'] = NULL;
    }

    $node->files = array();

    if (variable_get('node_export_reset_'. $node->type, FALSE)) {
      $node_options = variable_get('node_options_'. $node->type, array('status', 'promote'));
      // Fill in the default values.
      foreach (array('status', 'moderate', 'promote', 'sticky', 'revision') as $key) {
        $node->$key = in_array($key, $node_options);
      }
    }
    // Let other modules do special fixing up.
    // The function signature is: hook_export_node_alter(&$node, $original_node, $method)
    // Where $method is either 'prepopulate' or 'save-edit'.
    drupal_alter("node_export_node", $node, $original_node, $mode);

    return $node;

}

/**
 *  build node code string recursively
 */
function node_export_node_encode($var, $iteration = 0){
  $tab = '';
  for ($i = 0; $i < $iteration; $i++) {
    $tab = $tab ."  ";
  }
  $iteration++;
  if (is_object($var)) {
    $var = (array)$var;
    $var['#_export_node_encode_object'] = '1';
  }
  if (is_array($var)) {
    $empty = empty($var);
    $code = "array(". ($empty ? '' : "\n");
    foreach ($var as $key => $value) {
      $out = $tab ."  '". $key ."' => ".node_export_node_encode($value, $iteration).",\n";
      drupal_alter('node_export_node_encode', $out, $tab, $key, $value, $iteration);
      $code .= $out;
    }
    $code .= ($empty ? '' : $tab) .")";
    return $code;
  } else {
    if (is_string($var)) {
      return "'". addslashes($var) ."'";
    }
    elseif (is_numeric($var)) {
      return $var;
    }
    elseif (is_bool($var)) {
      return ($var ? 'TRUE' : 'FALSE');
    }
    else {
      return 'NULL';
    }
  }
}

/**
 *  eval and return decoded string
 */
function node_export_node_decode($string) {
  $array = eval('return '.$string.';');
  $return = node_export_node_decode_objects($array);
  return $return;
}

/**
 *  recursively convert arrays back to objects
 */
function node_export_node_decode_objects($array) {
  foreach ($array as $k => $v) {
    if (is_array($v)) {
      $array[$k] = node_export_node_decode_objects($v);
    }
    if (is_string($v)) {
      $array[$k] = stripslashes($v);
    }
  }
  if ($array['#_export_node_encode_object']) {
    unset($array['#_export_node_encode_object']);
    $array = (object)$array;
  }
  return $array;
}

/**
 * Create a new menu entry with title, parent and weight exported from
 * another nodes menu. Returns NULL if the node has no menu title.
 */
function node_export_get_menu($node) {
  // This will fetch the existing menu item if the node had one.
  node_invoke_nodeapi($node, 'prepare');
  if (!empty($node->menu['link_title'])) {
    $old_menu = $node->menu;
    // Now fetch the defaults for a new menu entry.
    $node = NULL;
    node_invoke_nodeapi($node, 'prepare');
    $node->menu['link_title'] = $old_menu['link_title'];
    $node->menu['plid'] = $old_menu['plid'];
    $node->menu['menu_name'] = $old_menu['menu_name'];
    $node->menu['weight'] = $old_menu['weight'];
    return $node->menu;
  }

}
