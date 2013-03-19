<?php

/**
 * @file
 *   Admin page callback file for the janrain_login module.
 */


/**
 * Menu callback: Generate a form to configure User entity to Engage mappings
 * used to pre-fill User entity fields with Engage data at signup and import
 * Engage profile data when a 3rd party account is linked (or user signs
 * in).
 *
 * @ingroup forms
 * @see janrain_login_mapping_settings_form_submit()
 */
function janrain_login_mapping_settings_form($form, &$form_state) {
  $map = variable_get('rpx_profile_fields_map', array());
  // Add empty mapping to the end of list.
  $map[] = array();

  $rpx_fields = db_query("SELECT fid, path FROM {rpx_profile_field}")->fetchAllAssoc('fid', PDO::FETCH_ASSOC);

  $group = 'mappings';
  foreach ($map as $mid => $mapping) {
    $form[$group][$mid] = array(
      'fid' => array(
        '#type' => 'select',
        '#options' => janrain_login_engage_field_options(),
        '#default_value' => isset($mapping['fid']) ? $mapping['fid'] : '',
      ),
      'separator' => array(
        '#markup' => 'maps to'
      ),
      'field' => array(
        '#type' => 'select',
        '#options' => janrain_login_user_field_options(),
        '#default_value' => isset($mapping['field']) ? $mapping['field'] : '',
      ),
    );
    if (isset($mapping['fid'])) {
      $edit = array(
        '#type' => 'link',
        '#title' => t('edit'),
        '#href' => "admin/config/people/janrain/mapping/edit/$mid",
      );
    }
    else {
      $edit = array(
        '#type' => 'markup',
        '#markup' => '&nbsp;',
      );
    }
    $form[$group][$mid]['edit'] = $edit;
  }

  $group = 'default_update_options';

  $form[$group] = array(
    '#type' => 'fieldset',
    '#title' => t('Social network precedence'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form[$group]['providers'] = array(
    '#theme' => 'janrain_login_provider_weight_table',
  );

  $providers = variable_get('rpx_default_provider_weight', array_keys(variable_get('rpx_providers')));
  _janrain_login_provider_table_generate($form[$group]['providers'], $providers);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));

  $form['#tree'] = TRUE;

  return $form;
}

/**
 * Submit handler to update changed Drupal to Engage data mapping.
 *
 * @see janrain_login_mapping_settings_form()
 */
function janrain_login_mapping_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $old_map = variable_get('rpx_profile_fields_map', array());
  $map = array();

  foreach ($values['mappings'] as $mid => $mapping) {
    // All field mappings should include a fid and a field.
    if(!empty($mapping['fid']) && !empty($mapping['field'])) {
      $map[$mid] = array(
        'fid' => $mapping['fid'],
        'set' => 'user',
        'field' =>  $mapping['field'],
        'update' => isset($old_map[$mid]['update']) ? $old_map[$mid]['update'] : RPX_UPDATE_NONE,
        'bundle' => 'user',
      );
      if (isset($old_map[$mid]['providers'])) {
        $map[$mid]['providers'] = $old_map[$mid]['providers'];
      }
    }
    // The mapping is dropped; we need to delete its records from the
    // rpx_mapping_provider table.
    else {
      db_delete('rpx_mapping_provider')
        ->condition('mid', $mid)
        ->execute();
    }
  }
  variable_set('rpx_profile_fields_map', $map);

  $provider_weight = _janrain_login_get_sorted_providers($form_state['values']['default_update_options']['providers']);
  variable_set('rpx_default_provider_weight', $provider_weight);

  drupal_set_message(t('Data mapping settings have been updated.'));
}

/**
 * Construct an array of options for field select inputs.
 *
 * @see janrain_login_mapping_settings_form()
 */
function janrain_login_engage_field_options() {
  $options = &drupal_static(__FUNCTION__);

  if (!isset($options)) {
    $options = array('' => '- Select a data field -');
    $fields = db_query("SELECT fid, title FROM {rpx_profile_field}");
    while ($field = $fields->fetchObject()) {
      $options[$field->fid] = $field->title;
    }
  }

  return $options;
}

/**
 * Construct an array of options for user field select inputs.
 *
 * @see janrain_login_mapping_settings_form()
 */
function janrain_login_user_field_options() {
  $options = &drupal_static(__FUNCTION__);

  if (!isset($options)) {
    $options = array('' => '- Select a user field -');
    foreach (field_info_instances('user', 'user') as $field_name => $field) {
      $options[$field_name] = $field['label'];
    }
  }

  return $options;
}

/**
 * Theme Engage field mapping form.
 *
 * @ingroup themeable
 */
function theme_janrain_login_mapping_settings_form($variables) {
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form['mappings']) as $key) {
    $field = &$form['mappings'][$key];

    // Add the row
    $row = array();
    $row[] = drupal_render($field['fid']);
    $row[] = drupal_render($field['separator']);
    $row[] = drupal_render($field['field']);
    $row[] = drupal_render($field['edit']);
    $rows[] = array('data' => $row);
  }
  $header = array(t('Social Network Field'), '', t('User Field'), t('Operations'));

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Menu callback: Generate a form to manage Engage profile fields.
 *
 * @ingroup forms
 */
function janrain_login_profile_field_overview_form($form, &$form_state) {
  $fields = db_query("SELECT fid, title, path FROM {rpx_profile_field}");
  while ($field = $fields->fetchObject()) {
    $admin_field_path = 'admin/config/people/janrain/fields/';
    $form[$field->fid] = array (
      'title' => array(
        '#markup' => $field->title,
      ),
      'path' => array(
        '#markup' => $field->path,
      ),
      'edit' => array(
        '#type' => 'link',
        '#title' => t('edit'),
        '#href' => $admin_field_path . 'edit/' . $field->fid,
        '#options' => array('attributes' => array('title' => t('Edit field.'))),
      ),
      'delete' => array(
        '#type' => 'link',
        '#title' => t('delete'),
        '#href' => $admin_field_path . 'delete/' . $field->fid,
        '#options' => array('attributes' => array('title' => t('Delete field.'))),
      ),
    );
  }

  // Additional row: add new field.
  $form['_edit_field'] = array(
    'title' => array(
      '#type' => 'textfield',
      '#title' => t('New field title'),
      '#title_display' => 'invisible',
      '#size' => 15,
      '#attributes' => array('class' => array('rpx-field-title-input')),
      '#description' => t('Title'),
      '#prefix' => '<div class="add-new-placeholder">' . t('Add new field') .'</div>',
    ),
    'path' => array(
      '#type' => 'textfield',
      '#title' => t('New field path'),
      '#title_display' => 'invisible',
      '#size' => 30,
      '#attributes' => array('class' => array('rpx-path-input')),
      '#description' => t('Janrain Engage data path'),
      '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
    ),
  );

  $path_help = '<dl>';
  $path_help .= '<dd>' . theme('rpx_path_tree') . '</dd>';
  $path_help .= '</dl>';

  $form['path_help'] = array(
    '#markup' => $path_help,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save new field'),
  );
  $form['actions']['reset'] = array(
    '#type' => 'link',
    '#title' => t('Reset to defaults'),
    '#href' => 'admin/config/people/janrain/fields/reset',
  );

  $form['#tree'] = TRUE;
  $form['#validate'][] = '_janrain_login_profile_field_forms_validate';
  $form['#submit'][] = '_janrain_login_profile_field_forms_submit';

  return $form;
}

/**
 * Menu callback: Generate a form to edit an Engage profile field.
 *
 * @ingroup forms
 */
function janrain_login_profile_field_edit_form($form, &$form_state, $arg = NULL) {
  if (is_numeric($arg)) {
    $fid = $arg;

    $field = db_query('SELECT fid, title, path FROM {rpx_profile_field} WHERE fid = :fid', array('fid' => $fid))->fetchAssoc();

    if (!$field) {
      drupal_not_found();
      drupal_exit();
    }
    drupal_set_title(t('Edit %title Engage field', array('%title' => $field['title'])), PASS_THROUGH);
    $form['_edit_field']['fid'] = array(
      '#type' => 'value',
      '#value' => $fid,
    );
  }
  else {
    drupal_not_found();
    drupal_exit();
  }

  $form['_edit_field']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $field['title'],
    '#attributes' => array('class' => array('rpx-field-title-input')),
    '#description' => t('The title of the field. The title is shown in the mapping form next to the data path. An example title is "Verified email". '),
  );
  $form['_edit_field']['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Engage data path'),
    '#default_value' => $field['path'],
    '#attributes' => array('class' => array('rpx-path-input')),
    '#description' => t("The path to the data within the Engage authentication dataset, in a PHP-like array referencing notation.
An example path is <code>['profile']['verifiedEmail']</code> or <code>['merged_poco']['emails'][0]['value']</code>. You can use the <a href=\"@provider-configuration\" target=\"_blank\">provider configuration</a> and <a href=\"@signin-test\" target=\"_blank\">sign-in test</a> tools to configure the data to be returned by providers, and to see it in action.", array('@provider-configuration' => variable_get('rpx_admin_url', '') . '/providers', '@signin-test' => variable_get('rpx_admin_url', '') . '/test')),
  );

  $path_help = '<dl>';
  $path_help .= '<dt>' . t('You can choose the path here. Please note that not all providers return all of the data below, and some of the data is only available to <a href="@get_engage" target="_blank">Plus</a> and <a href="@get_engage" target="_blank">Pro</a> Janrain customers.  For complete information about the user data returned by Engage see <a href="@auth_info_help" target="_blank">this</a>.', array('@get_engage' => url('http://www.janrain.com/products/engage/get-janrain-engage'), '@settings' => url('/admin/config/people/janrain'), '@auth_info_help' => 'http://documentation.janrain.com/engage/api/auth_info')) . '</dt>';
  $path_help .= '<dd>' . theme('rpx_path_tree') . '</dd>';
  $path_help .= '</dl>';
  $form['path_help'] = array(
    '#markup' => $path_help,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save field'),
  );

  $form['#tree'] = TRUE;

  $form['#validate'][] = '_janrain_login_profile_field_forms_validate';
  $form['#submit'][] = '_janrain_login_profile_field_forms_submit';

  return $form;
}

/**
 * Validate janrain_login_profile_field_overview_form and janrain_login_profile_field_edit_form submissions.
 */
function _janrain_login_profile_field_forms_validate($form, &$form_state) {
  $field = $form_state['values']['_edit_field'];

  // Missing title.
  if (!$field['title']) {
    form_set_error('_edit_field][title', t('You need to provide a title.'));
  }
  // Missing data path.
  if (!$field['path']) {
    form_set_error('_edit_field][path', t('You need to provide a data path.'));
  }

  // Validate the data path by parsing it, also save parsed path to values.
  $parsed_path = _janrain_login_parse_path($field['path']);
  if ($parsed_path) {
    $form_state['values']['_edit_field']['parsed_path'] = $parsed_path;
  }
  else {
    _janrain_login_profile_path_validate_error();
  }

  $query = db_select('rpx_profile_field');
  $query->fields('rpx_profile_field', array('fid'));

  if (isset($field['fid'])) {
    $query->condition('fid', $field['fid'], '<>');
  }
  $query_path = clone $query;

  $title = $query
    ->condition('title', $field['title'])
    ->execute()
    ->fetchField();
  if ($title) {
    form_set_error('_edit_field][title', t('The specified title is already in use.'));
  }
  $path = $query_path
    ->condition('path', $field['path'])
    ->execute()
    ->fetchField();
  if ($path) {
    form_set_error('_edit_field][path', t('The specified path is already in use.'));
  }
}

/**
 * Process janrain_login_profile_field_overview and janrain_login_profile_field_form submissions.
 */
function _janrain_login_profile_field_forms_submit($form, &$form_state) {
  $field = $form_state['values']['_edit_field'];

  // Remove all elements that are not rpx_profile_field columns.
  $values = array_intersect_key($field, array_flip(array('title', 'path', 'parsed_path')));
  if (!isset($field['fid'])) {
    db_insert('rpx_profile_field')
      ->fields(array('title' => $values['title'], 'path' => $values['path'], 'parsed_path' => serialize($values['parsed_path'])))
      ->execute();
    drupal_set_message(t('The field has been created.'));
    watchdog('janrain_login', 'Engage profile field %field added with path %path.', array('%field' => $field['title'], '%path' => $field['path']), WATCHDOG_NOTICE, l(t('view'), 'admin/config/people/janrain/fields'));
  }
  else {
    db_update('rpx_profile_field')
      ->fields(array('title' => $values['title'], 'path' => $values['path'], 'parsed_path' => serialize($values['parsed_path'])))
      ->condition('fid', $field['fid'])
      ->execute();
    drupal_set_message(t('The field has been updated.'));
  }
  cache_clear_all();
  menu_rebuild();

  $form_state['redirect'] = 'admin/config/people/janrain/fields';
  return;
}

function _janrain_login_profile_path_validate_error() {
  form_set_error('_edit_field][path', t('The specified path is not valid. Please check that it does not contain spaces or any other special characters except brackets ([]) and apostrophe (\'), and it would make a valid PHP array reference if appended to an array variable.'));
}

/**
 * Returns HTML for the Engage fields overview page.
 *
 * @ingroup themeable
 */
function theme_janrain_login_profile_field_overview_form($variables) {
  drupal_add_css(drupal_get_path('module', 'janrain_client') . '/janrain_client.css');
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form) as $key) {
    // Skip form control elements.
    if (array_key_exists('path', $form[$key])) {
      $field = &$form[$key];

      // Add the row
      $row = array();
      $row[] = drupal_render($field['title']);
      $row[] = drupal_render($field['path']);
      $row[] = drupal_render($field['edit']);
      $row[] = drupal_render($field['delete']);
      $rows[] = array('data' => $row);
    }
  }

  $header = array(t('Title'), t('Path'));
  $header[] = array('data' => t('Operations'), 'colspan' => 2);

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Provide a 'tree' display of RPX data paths.
 * Based on code from the Token module.
 *
 * @ingroup themeable
 */
function theme_rpx_path_tree($variables) {
  module_load_include('inc', 'janrain_login', 'janrain_login.paths');

  $header = array(
    t('Name'),
    t('Path'),
    t('Description'),
  );

  $rows = _rpx_paths();

  // On rare occasions, custom JS is required instead of the default.  There is no
  // UI for changing this var. It is assumed if this var is set to FALSE, the developer
  // handles inclusion of equivalent JS.
  if (variable_get('janrain_login_add_default_js', TRUE)) {
    drupal_add_js(drupal_get_path('module', 'janrain_client') . '/janrain_client.js');
  }
  drupal_add_css(drupal_get_path('module', 'janrain_client') . '/janrain_client.css');

  $table_options = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('rpx-path-tree')),
    'empty' => t('No paths available.'),
  );
  $table_options['caption'] = t('To insert a token, click the Path text box and then click the token.');
  $table_options['attributes']['class'][] = 'rpx-path-click-insert';

  return theme('tree_table', $table_options);
}

/**
 * Extract path components.
 *
 * @param string $path
 * @return array
 */
function _janrain_login_parse_path($path) {
  $path = trim($path, '[]');
  $parts = explode('][', $path);
  if (!empty($parts)) {
    $parsed = array();
    foreach ($parts as $part) {
      if (is_numeric($part)) {
        $parsed[] = intval($part);
      }
      elseif (preg_match("/^\'([a-z0-9_]+)\'$/i", $part, $matches)) {
        $parsed[] = $matches[1];
      }
      else {
        return NULL;
      }
    }
    return $parsed;
  }
  else {
    return NULL;
  }
}

/**
 * Menu callback: Generate a form to edit an Engage to Drupal mapping.
 *
 * @ingroup forms
 * @see janrain_login_profile_mapping_edit_form_submit()
 */
function janrain_login_profile_mapping_edit_form($form, &$form_state, $arg = NULL) {
  $map = variable_get('rpx_profile_fields_map', array());
  if (is_numeric($arg)) {
    $mid = $arg;

    if (!isset($map[$mid]['fid'])) {
      drupal_not_found();
      drupal_exit();
    }

    // Set title for the edit form.
    $field = db_query("SELECT title, path FROM {rpx_profile_field} WHERE fid = :fid", array(':fid' => $map[$mid]['fid']))->fetchObject();
    drupal_set_title(t('Edit mapping for field %title (%path)', array('%title' => $field->title, '%path' => $field->path)), PASS_THROUGH);

    $form['mid'] = array(
      '#type' => 'value',
      '#value' => $mid,
    );
  }
  else {
    drupal_not_found();
    drupal_exit();
  }

  $form['field_update'] = array(
   '#type' => 'fieldset',
   '#title' => t('Field update options for the mapping'),
  );
  $form['field_update']['options'] = array(
    '#type' => 'radios',
    '#title' => t('Data mapping logic for 3rd party accounts'),
    '#default_value' => isset($map[$mid]['update']) ? $map[$mid]['update'] : RPX_UPDATE_NONE,
    '#options' => array(
      RPX_UPDATE_NONE => t('Do not update the data field upon user login (default)'),
      RPX_UPDATE_EMPTY => t('Only update the data field if it is empty at user login'),
      RPX_UPDATE_ALWAYS => t('Update the data field (including overwriting existing data) at user login'),
      RPX_UPDATE_ADD => t('Update the data field as an additional value (but do not overwrite existing data) at user login (this will only work for fields that allow multiple values)'),
      RPX_UPDATE_MAYBE => t('Update the data field (including overwriting existing data) only when a higher priority provider is used to login (use the table below to set priorities)'),
    ),
  );
  $form['field_update']['providers'] = array(
      '#theme' => 'janrain_login_provider_weight_table',
  );

  $providers = isset($map[$mid]['providers']) ? $map[$mid]['providers'] : variable_get('rpx_default_provider_weight', array_keys(janrain_client_get_providers(TRUE)));

  _janrain_login_provider_table_generate($form['field_update']['providers'], $providers);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#tree'] = TRUE;

  return $form;
}

/**
 * Submit handler to update Engage to Drupal mapping.
 *
 * @see janrain_login_profile_mapping_edit_form()
 */
function janrain_login_profile_mapping_edit_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $map = variable_get('rpx_profile_fields_map', array());
  $mapping = &$map[$values['mid']];

  $update = $values['field_update']['options'];
  $mapping['update'] = $update;

  if ($update == RPX_UPDATE_MAYBE) {
    $mapping['providers'] = _janrain_login_get_sorted_providers($values['field_update']['providers']);
  }

  variable_set('rpx_profile_fields_map', $map);

  // Warn user if he tries to configure the mapping to append new data to a
  // single value field.
  if ($update == RPX_UPDATE_ADD) {
    $field_info = field_info_field($mapping['field']);
    if (!isset($field_info['cardinality']) || $field_info['cardinality'] == 1) {
      drupal_set_message(t('The field cannot accept multiple values.'), 'warning');
    }
  }

  // Get title and path for the field that is being mapped, so that we can
  // print a friendlier message.
  $field = db_query("SELECT title, path FROM {rpx_profile_field} WHERE fid = :fid", array(':fid' => $mapping['fid']))->fetchObject();
  drupal_set_message(t('Mapping options for field %title (%path) have been updated.', array('%title' => $field->title, '%path' => $field->path)));
  $form_state['redirect'] = 'admin/config/people/janrain/mapping';
}

/**
 * Helper function: creates a provider weight table within a form element.
 */
function _janrain_login_provider_table_generate(&$container, $providers) {
  $entire_list = TRUE;
  $provider_data = janrain_client_get_providers($entire_list);

  $order = -10;
  foreach ($providers as $provider) {
    $container[$provider]['title'] = array(
      '#type' => 'item',
      '#markup' =>  theme('rpx_icon', array('provider' => $provider, 'style' => 'rpx-icon-inline')) . '<span' . drupal_attributes(array('class' => 'rpx-provider-title')) . '>' . $provider_data[$provider] .  '</span>',
    );
    $container[$provider]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @provider', array('@provider' => $provider)),
      '#title_display' => 'invisible',
      '#default_value' => $order,
    );
    $order++;
  }
}

/**
 * Helper function: returns an array of providers sorted according to their
 * weight in the provider weight table.
 */
function _janrain_login_get_sorted_providers($providers) {
  foreach($providers as $k => $v) {
    $providers[$k] = $providers[$k]['weight'];
  }
  asort($providers);
  return array_keys($providers);
}

/**
 * Returns HTML for the provider weight table.
 *
 * @ingroup themeable
 */
function theme_janrain_login_provider_weight_table($variables) {
  $providers = $variables['providers'];

  $rows = array();
  foreach (element_children($providers) as $provider) {
    $providers[$provider]['weight']['#attributes']['class'] = array('rpx-mapping-provider-weight');
    $row = array();
    $row[] = drupal_render($providers[$provider]['title']);
    $row[] = drupal_render($providers[$provider]['weight']);
    $rows[] = array(
      'data' => $row,
      'class' => array('draggable'),
    );
  }
  $header = array(t('Authorized Social Network'), t('Weight'));
  drupal_add_tabledrag('rpx-mapping-providers', 'order', 'sibling', 'rpx-mapping-provider-weight');
  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'rpx-mapping-providers'), 'caption' => 'Arrange social networks from highest priority (top) to lowest. Data from higher priority social networks takes precedence over lower priorities.'));
}

/**
 * Menu callback; deletes a Engage field from the Drupal to Engage mapping
 * configuration.
 */
function janrain_login_profile_field_delete($form, &$form_state, $fid = NULL) {
  $field = db_query("SELECT title FROM {rpx_profile_field} WHERE fid = :fid", array(':fid' => $fid))->fetchObject();
  if (!$field) {
    drupal_not_found();
    drupal_exit();
  }
  $form['fid'] = array('#type' => 'value', '#value' => $fid);
  $form['title'] = array('#type' => 'value', '#value' => $field->title);

  return confirm_form($form,
    t('Are you sure you want to delete the Engage field %field?', array('%field' => $field->title)), 'admin/config/people/janrain/fields',
    t('This action cannot be undone. The Drupal to Engage data mappings using this field will be deleted as well.'),
    t('Delete'), t('Cancel'));
}

/**
 * Process an Engage field delete form submission.
 */
function janrain_login_profile_field_delete_submit($form, &$form_state) {
  db_delete('rpx_profile_field')
    ->condition('fid', $form_state['values']['fid'])
    ->execute();

  // Remove the mappings (if any) that use the field, also deleting their
  // records from the rpx_mapping_provider table.
  $map = variable_get('rpx_profile_fields_map', array());
  foreach ($map as $mid => $mapping) {
    if ($mapping['fid'] == $form_state['values']['fid']) {
      unset($map[$mid]);
      db_delete('rpx_mapping_provider')
        ->condition('mid', $mid)
        ->execute();
    }
  }
  variable_set('rpx_profile_fields_map', $map);

  cache_clear_all();

  drupal_set_message(t('The Engage field %field has been deleted.', array('%field' => $form_state['values']['title'])));
  watchdog('janrain_login', 'Engage field %field deleted.', array('%field' => $form_state['values']['title']), WATCHDOG_NOTICE, l(t('view'), 'admin/config/people/janrain/fields'));

  $form_state['redirect'] = 'admin/config/people/janrain/fields';
  return;
}

/**
 * Menu callback; reset fields confirmation form.
 */
function janrain_login_profile_field_reset_confirm($form, &$form_state) {
  return confirm_form($form, t('Are you sure you want to revert to the default Janrain Engage profile fields?'), 'admin/config/people/janrain/fields/', t('Any new fields that you have created will be lost. Any existing mappings will be edelted as well. This action cannot be undone.'), t('Reset'));
}

/**
 * Process fields reset form submissions.
 *
 * Reset Engage fields by deleting all existing fields and creating default ones
 * (those that are created at module installation). As fid's are going to
 * change, also delete mappings and their records in the rpx_mapping_provider
 * table.
 */
function janrain_login_profile_field_reset_confirm_submit($form, &$form_state) {
  db_query("DELETE FROM {rpx_profile_field}");

  _janrain_login_profile_field_ensure_defaults();

  // Delete field map.
  variable_set('rpx_profile_fields_map', array());

  // Delete data provider records.
  db_delete('rpx_mapping_provider')
    ->execute();

  drupal_set_message(t('The Janrain Engage profile fields have been reverted to the defaults.'));
  $form_state['redirect'] = 'admin/config/people/janrain/fields/';
}

/**
 * Inserts default values into rpx_profile_field, or updates existing values.
 */
function _janrain_login_profile_field_ensure_defaults() {
  $fields = array(
    array('First Name', "['profile']['name']['givenName']"),
    array('Last Name', "['profile']['name']['familyName']"),
    array('Display Name', "['profile']['displayName']"),
    array('Gender', "['profile']['gender']"),
    array('Birthday', "['profile']['birthday']"),
    array('Phone Number', "['profile']['phoneNumber']"),
    array('Street Address', "['profile']['address']['streetAddress']"),
    array('City', "['profile']['address']['locality']"),
    array('State/Province', "['profile']['address']['region']"),
    array('Zipcode/Postal Code', "['profile']['address']['postalCode']"),
    array('Country', "['profile']['address']['country']"),
  );
  foreach ($fields as $field) {
    db_merge('rpx_profile_field')
      ->key(array('title' => $field[0]))
      ->fields(array('path' => $field[1], 'parsed_path' => serialize(_janrain_login_parse_path($field[1]))))
      ->execute();
  }
}

/**
 * Helper function: rebuild all provider precedence lists so that they have a
 * provider in them iff that provider is in the authorized list.
 */
function janrain_login_rebuild_provider_precedence_lists() {
  if($providers = variable_get('rpx_default_provider_weight', NULL)) {
    $providers = janrain_login_update_ordered_provider_list($providers);
    variable_set('rpx_default_provider_weight', $providers);
  }
  if($map = variable_get('rpx_profile_fields_map', NULL)) {
    foreach($map as &$mapping) {
      if(isset($mapping['providers'])) {
        $mapping['providers'] = janrain_login_update_ordered_provider_list($mapping['providers']);
      }
    }
    variable_set('rpx_profile_fields_map', $map);
  }
}

/**
 * Helper function: return a list of all authorized providers sorted in
 * accordance with the precedence array (passed as an argument).
 *
 * Preserve existing order of the precedence array by culling
 * de-authorized providers from the array and appending newly
 * authorized providers to its end.
 */
function janrain_login_update_ordered_provider_list($original) {
  $authorized = &drupal_static(__FUNCTION__);

  if (!isset($authorized)) {
    $authorized = array_keys(variable_get('rpx_providers'));
  }

  $result = array_intersect($original, $authorized);
  $result = array_merge($result, array_diff($authorized, $result));

  return $result;
}