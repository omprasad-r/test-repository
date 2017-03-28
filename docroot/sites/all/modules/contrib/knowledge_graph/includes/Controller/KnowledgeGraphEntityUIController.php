<?php
/**
 * UI controller.
 */
class KnowledgeGraphEntityUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = array();
    // Set this on the object so classes that extend hook_menu() can use it.
    $this->id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ? $this->entityInfo['admin ui']['menu wildcard'] : '%knowledge_graph';
    $plural_label = isset($this->entityInfo['plural label']) ? $this->entityInfo['plural label'] : $this->entityInfo['label'] . 's';

    $items[$this->path] = array(
      'title' => $plural_label,
      'page callback' => 'drupal_get_form',
      'page arguments' => array($this->entityType . '_overview_form', $this->entityType),
      'description' => 'Manage ' . $plural_label . '.',
      'access callback' => 'entity_access',
      'access arguments' => array('view', $this->entityType),
      'file' => 'includes/entity.ui.inc',
    );
    $items[$this->path . '/list'] = array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
    );
    $items[$this->path . '/add'] = array(
      'title callback' => 'entity_ui_get_action_title',
      'title arguments' => array('add', $this->entityType),
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, NULL, 'add'),
      'access callback' => 'entity_access',
      'access arguments' => array('create', $this->entityType),
      'type' => MENU_LOCAL_ACTION,
    );
    $items[$this->path . '/manage/' . $wildcard] = array(
      'title' => 'Edit',
      'title callback' => 'entity_label',
      'title arguments' => array($this->entityType, $this->id_count + 1),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('knowledge_graph_form', $this->id_count + 1),
      'access callback' => 'entity_access',
      'access arguments' => array('update', $this->entityType, $this->id_count + 1),
    );
    $items[$this->path . '/manage/' . $wildcard . '/edit'] = array(
      'title' => 'Edit',
      'load arguments' => array($this->entityType),
      'type' => MENU_DEFAULT_LOCAL_TASK,
    );

    // Clone form, a special case for the edit form.
    $items[$this->path . '/manage/' . $wildcard . '/clone'] = array(
      'title' => 'Clone',
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, $this->id_count + 1, 'clone'),
      'load arguments' => array($this->entityType),
      'access callback' => 'entity_access',
      'access arguments' => array('create', $this->entityType),
    );
    // Menu item for operations like revert and delete.
    $items[$this->path . '/manage/' . $wildcard . '/%'] = array(
      'page callback' => 'drupal_get_form',
      'page arguments' => array($this->entityType . '_operation_form', $this->entityType, $this->id_count + 1, $this->id_count + 2),
      'load arguments' => array($this->entityType),
      'access callback' => 'entity_access',
      'access arguments' => array('delete', $this->entityType, $this->id_count + 1),
      'file' => 'includes/entity.ui.inc',
    );

    if (!empty($this->entityInfo['exportable'])) {
      // Menu item for importing an entity.
      $items[$this->path . '/import'] = array(
        'title callback' => 'entity_ui_get_action_title',
        'title arguments' => array('import', $this->entityType),
        'page callback' => 'drupal_get_form',
        'page arguments' => array($this->entityType . '_operation_form', $this->entityType, NULL, 'import'),
        'access callback' => 'entity_access',
        'access arguments' => array('create', $this->entityType),
        'file' => 'includes/entity.ui.inc',
        'type' => MENU_LOCAL_ACTION,
      );
    }

    if (!empty($this->entityInfo['admin ui']['file'])) {
      // Add in the include file for the entity form.
      foreach (array("/manage/$wildcard", "/manage/$wildcard/clone", '/add') as $path_end) {
        $items[$this->path . $path_end]['file'] = $this->entityInfo['admin ui']['file'];
        $items[$this->path . $path_end]['file path'] = isset($this->entityInfo['admin ui']['file path']) ? $this->entityInfo['admin ui']['file path'] : drupal_get_path('module', $this->entityInfo['module']);
      }
    }
    return $items;
  }


  /**
   * Overrides entity_ui_overview_form().
   */
  public function overviewTable($conditions = array()) {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', $this->entityType);

    // Add all conditions to query.
    foreach ($conditions as $key => $value) {
      $query->propertyCondition($key, $value);
    }

    if ($this->overviewPagerLimit) {
      $query->pager($this->overviewPagerLimit);
    }

    $results = $query->execute();

    $ids = isset($results[$this->entityType]) ? array_keys($results[$this->entityType]) : array();
    $entities = $ids ? entity_load($this->entityType, $ids) : array();
    ksort($entities);

    $rows = array();
    foreach ($entities as $entity) {
      $rows[] = $this->overviewTableRow($conditions, entity_id($this->entityType, $entity), $entity);
    }
    $render = array(
      '#theme' => 'table',
      '#header' => $this->overviewTableHeaders($conditions, $rows),
      '#rows' => $rows,
      '#empty' => t('None.'),
    );
    return $render;
  }
}


/**
 * @file
 *  Contains form and helper functions to create and edit a knowledge_graph entity.
 */

/**
 * Form to add new mappings.
 * @param entity $knowledge_graph_type
 *   The scheme to be mapped.
 */
function knowledge_graph_form($form, &$form_state, $knowledge_graph_type) {

  if ($knowledge_graph_type->entityType() == "knowledge_graph") {
    $knowledge_graph = $knowledge_graph_type;
    $knowledge_graph_type = knowledge_graph_type_load($knowledge_graph_type->type);
    $form['knowledge_graph'] = array('#type' => 'value', '#value' => $knowledge_graph);
  }
  // Get fields from the current knowledge graph type.
  $knowledge_graph_type_fields = field_info_instances('knowledge_graph', $knowledge_graph_type->type);

  // Add fields to be directly entered instead of be mapped.
  $knowledge_graph_type_direct_fields = array();
  // Add fields for mapping.
  $knowledge_graph_type_map_fields = array();
  
  // Only add fields of type "field_knowledge_graph_mapper" to the options.
  // Other fields will be rendered directly.
  foreach ($knowledge_graph_type_fields as $value => $knowledge_graph_type_field) {
    if ($knowledge_graph_type_field['widget']['type'] == 'knowledge_graph_mapper') {
      $knowledge_graph_type_map_fields[$value] = $knowledge_graph_type_field;
    }
    else {
      $knowledge_graph_type_direct_fields[$value] = $knowledge_graph_type_field;
    }
  }
  // Sort fields by it's display order.
  $field_weight = array();
  foreach ($knowledge_graph_type_map_fields as $key => $value) {
     $field_weight[$key] = !empty($value['widget']['weight']) ? $value['widget']['weight'] : 0;
  }
  array_multisort($field_weight, SORT_ASC, $knowledge_graph_type_map_fields);

  // Save values to be used later.
  $form['knowledge_graph_type'] = array('#type' => 'value', '#value' => $knowledge_graph_type);
  $form['knowledge_graph_type_fields'] = array('#type' => 'value', '#value' => $knowledge_graph_type_map_fields);
  // Get all node bundles.
  $bundles = field_info_bundles('node');
  $bundle_options = array('' => t('Please select a bundle.'));
  // Filter bundles - only show bundles without mapping.
  if (empty($form['knowledge_graph'])) {
    $bundles = _knowledge_graph_filter_bundles($bundles, $knowledge_graph_type->type);
  }  
  foreach ($bundles as $value => $bundle) {
    $bundle_options[$value] = $bundle['label'];
  }

  // Add select to let the user choose which bundle should be mapped.
  $form['bundles'] = array(
    '#title' => t('Select the bundle you want to map.'),
    '#description' => t('If you cannot select any bundle here all bundles are already mapped to type !type.', array('!type' => '<strong>' . $knowledge_graph_type->label . '</strong>')),
    '#type' => 'select',
    '#options' => $bundle_options,
    '#required' => TRUE,
    '#default_value' => (isset($knowledge_graph->bundle_ref) ? $knowledge_graph->bundle_ref : ''),
    '#ajax' => array(
      'wrapper' => 'knowledge_bundle_fields',
      'callback' => '_ajax_replace_bundle_fields_callback',
      'method' => 'replace',
    ),
  );
  // Read-only for bundles field while editing existing bundle.
  if (!empty($form['knowledge_graph'])) {
    $form['bundles']['#disabled'] = TRUE;
  }
  
  // Load fields which will be entered directly in the form.
  $form['direct_fields'] = array(
    '#title' => t("Direct fields"),
    '#type' => 'fieldset',
  );
  foreach ($knowledge_graph_type_direct_fields as $field_name => $field_definition) {
    // Load the values from the graph if existing.
    if (isset($knowledge_graph)) {
      knowledge_graph_mapping_add_attach_form($field_name, 'knowledge_graph', $knowledge_graph->type, $knowledge_graph, $form, $form_state);
    }
    else {
      knowledge_graph_mapping_add_attach_form($field_name, 'knowledge_graph', $knowledge_graph_type->type, $knowledge_graph_type, $form, $form_state);
    }
  }

  // Placeholder to be replaced on ajax call.
  $form['fields'] = array(
    '#title' => t("Fields to be mapped"),
    '#prefix' => '<div id="knowledge_bundle_fields">',
    '#suffix' => '</div>',
    '#type' => 'hidden',
    '#description' => t('Please choose how the fields should be mapped.'),
  );
  // Add fields of ctype after ajax callback.
  if (!empty($form_state['values']['bundles'])) {
    $fields = field_info_instances('node', check_plain($form_state['values']['bundles']));
  }
  if (empty($form_state['values']['bundles']) && !empty($knowledge_graph->bundle_ref) ){
    $fields = field_info_instances('node', $knowledge_graph->bundle_ref);
  }

  if (!empty($fields)) {
    // Get fields from the bundle.
    $params = array(
      'knowledge_graph' => $knowledge_graph,
      'knowledge_graph_type_map_fields' => $knowledge_graph_type_map_fields,
      'knowledge_graph_fields' => $fields,
    );
    knowledge_graph_get_field_form($form, $params);
  }
  else {
    // Placeholder to be replaced on ajax call.
    $form['fields'] = array(
      '#title' => t("Fields to be mapped"),
      '#prefix' => '<div id="knowledge_bundle_fields">',
      '#suffix' => '</div>',
      '#type' => 'hidden',
      '#description' => t('Please choose how the fields should be mapped.'),
    );
  }
  ksort($form['fields']);
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save map'),
    '#weight' => 40,
  );

  // Attach css for styling.
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'knowledge_graph') . '/styles/knowledge_graph.css',
  );
  return $form;
}

/**
 * Validate the knowledge_graph_mapping_add_form.
 */
function knowledge_graph_form_validate($form, &$form_state) {
  if (empty($form_state['values']['knowledge_graph_type'])) {
    form_set_error('knowledge_graph_type', t('Could not find knowledge graph type'));
  }
}

/**
 * Submit the knowledge_graph_mapping_add_form.
 */
function knowledge_graph_form_submit($form, &$form_state) {
  $knowledge_graph_type = $form_state['values']['knowledge_graph_type'];
  $knowledge_graph = isset($form_state['values']['knowledge_graph']) ? $form_state['values']['knowledge_graph'] : '';
  // Fields of the mapper which are supposed to map fields later.
  $mapping_fields = $form_state['values']['knowledge_graph_type_fields'];

  // Fields of the mapper which have direct input.
  $direct_fields = $form_state['field'];

  if (!empty($knowledge_graph)) {
    try {
      $entity = entity_metadata_wrapper('knowledge_graph', $knowledge_graph);
      $message = t('Knowledge graph entity updated successfully.'); 
    }
    catch (EntityMetadataWrapperException $e){
      drupal_set_message("Could not edit knowledge_graph entity", 'error');
    }
  }
  else {
    // Create new entity and save it.
    $entity_new = entity_create('knowledge_graph', array(
        'type' => $knowledge_graph_type->type,
        'name' => "Map " . $knowledge_graph_type->type . '/' . $form_state['values']['bundles'],
        'entity_type_ref' => 'node',
        'bundle_ref' => check_plain($form_state['values']['bundles']),
      )
    );
    $entity = entity_metadata_wrapper('knowledge_graph', $entity_new);
    $message = t('Knowledge graph entity saved successfully.');
  }
  // Save the values for our direct fields.
  foreach ($direct_fields as $field_name => $direct_field) {
    // We need to add a separate handling for list fields to get the value of the selected item.
    $field = current($direct_field);
    $field = $field['field'];
    if ($field['module'] == "list") {
      $field_settings = $field['settings'];
      $entity->{$field_name} = $field_settings['allowed_values'][$form_state['values'][$field_name][LANGUAGE_NONE][0]['value']];
    }
    else {
      $entity->{$field_name} = $form_state['values'][$field_name][LANGUAGE_NONE][0]['value'];
    }
  }
  // Save all mappings for the mapping fields.
  foreach ($mapping_fields as $mapping_fieldname => $mapping_fieldlabel) {
    if (!empty($mapping_fieldname)) {
      $entity->{$mapping_fieldname}  = $form_state['values'][$mapping_fieldname];
    }
  }
  if (entity_save('knowledge_graph', $entity)) {
    // Show message ad redirect user after saving entity.
    drupal_set_message($message);
    $form_state['redirect'] = 'admin/structure/knowledge_graph_maps';
  }
}

/**
 * Attach a field to the add form.
 */
function knowledge_graph_mapping_add_attach_form($field_name, $entity_type, $bundle, $entity, &$form, &$form_state, $langcode = NULL) {
  // Set #parents to 'top-level' if it doesn't exist.
  $form += array('#parents' => array());

  // If no language is provided use the default site language.
  $options = array(
    'language' => field_valid_language($langcode),
    'default' => TRUE,
  );

  // Append to the form
  ctools_include('fields');
  $field_instance = field_info_instance($entity_type, $field_name, $bundle);
  if (module_exists('field_group')) {
    $field_groups = field_group_info_groups($entity_type, $bundle);
    if (!empty($field_groups['form'])) {
      foreach ($field_groups['form'] as $field_group_name => $field_groups_settings) {
        if (in_array($field_name, $field_groups_settings->children)) {
          $group = explode("group_", $field_group_name, 2);
          $field_instance['label'] .= " (" . $group[1] . '.' . $field_instance['label']  . ")";
        }
      }
    }
  }
  $form['direct_fields'] += (array) ctools_field_invoke_field($field_instance, 'form', $entity_type, $entity, $form, $form_state, $options);
}

function knowledge_graph_mapping_delete_form($form, &$form_state, $entity) {
  $form['#entity'] = $entity;
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $entity->name)),
    "admin/structure/knowledge_graph/",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function knowledge_graph_mapping_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $entity = $form['#entity'];
    entity_delete('knowledge_graph', $entity->id);
    cache_clear_all();
    watchdog('content', '@type: deleted %title.', array('@type' => $entity->type, '%title' => $entity->name));
    drupal_set_message(t('@type %title has been deleted.', array('@type' => $entity->type, '%title' => $entity->name)));
  }

  $form_state['redirect'] = '<front>';
}

function knowledge_graph_get_field_form(&$form, $params = array()) {
  // Get field group info for page.
  if (module_exists('field_group')) {
     $bundle = $form['knowledge_graph_type']['#value']->type;
     $field_groups = field_group_info_groups('knowledge_graph', $bundle);
  }
  // Build options array.
  $bundle_fields_option = array('' => '<none>');
  // Add properties of the node.
  $properties = entity_get_property_info('node');
  foreach ($properties['properties'] as $property => $property_settings) {
    $bundle_fields_option["prop_" . $property] = $property;
  }
  // Get all field information into the field options.
  $bundle_fields_option = $bundle_fields_option + _knowledge_graph_build_bundle_field_options($params['knowledge_graph_fields']);
  
  foreach ($params['knowledge_graph_type_map_fields'] as $knowledge_graph_type_field_value => $knowledge_graph_type_field) {
    $group = $knowledge_graph_type_field['settings']['group'];
    $property = $knowledge_graph_type_field['settings']['property'];
    // Add grouping for fields.
    if (!empty($group) && !empty($field_groups)) {
      // Create group if not available.
      if (!isset($form['fields'][$group])) {
        $form['fields'][$group] = array(
          '#type' => 'fieldset',
          '#title' => $group,
          '#weight' => $field_groups['form']['group_' . $group]->weight - 0.5,
        );
      }
      $form['fields'][$group][$knowledge_graph_type_field_value] = array(
        '#title' => t('Field: ' . $knowledge_graph_type_field['label'] . ' (' . $property .')'),
        '#description' => isset($knowledge_graph_type_field['description']) ? t($knowledge_graph_type_field['description']) : '',
        '#type' => 'select',
        '#options' => $bundle_fields_option,
        '#default_value' => isset($params['knowledge_graph']->{$knowledge_graph_type_field_value}) ? $params['knowledge_graph']->{$knowledge_graph_type_field_value}[LANGUAGE_NONE][0]['field_name'] : '',
        '#weight' => $knowledge_graph_type_field['widget']['weight'],
      );
    } else {
      $form['fields'][$knowledge_graph_type_field_value] = array(
        '#title' => t('Field: ' . $knowledge_graph_type_field['label'] . ' (' . $property .')'),
        '#description' => isset($knowledge_graph_type_field['description']) ? t($knowledge_graph_type_field['description']) : '',
        '#type' => 'select',
        '#options' => $bundle_fields_option,
        '#default_value' => isset($params['knowledge_graph']->{$knowledge_graph_type_field_value}) ? $params['knowledge_graph']->{$knowledge_graph_type_field_value}[LANGUAGE_NONE][0]['field_name'] : '',
        '#weight' => $knowledge_graph_type_field['widget']['weight'],
      );
    }
  }
  $form['fields']['#type'] = 'fieldset';
}

/**
 * Build the option menu for the field mapper.
 * @param $fields
 *  The fields to be added to the option array.
 * @param bool $load_ref
 *  Whether entity reference fields be loaded and the fields of the target be added to the options.
 * @param int $level
 *  The recursion level.
 * @param $parent_field
 *  The reference field of the reference entity.
 * @return mixed
 */
function _knowledge_graph_build_bundle_field_options($fields, $load_ref = TRUE, $level = 0, $parent_field = "") {
  $level_string = str_repeat("-", $level);
  if (!empty($parent_field)) {
    $parent_field = "ref_" . $parent_field . '_';
  }
  foreach ($fields as $value => $field) {
    $bundle_fields_option[$value] =$level_string . ' ' . $field['label'];
    $field_info = field_info_field($value);
    // Add columns of field to the options.
    foreach ($field_info['columns'] as $column_name => $column_settings) {
      // Simply add the column to the option array if reference fields should not be processed.
      if (!$load_ref) {
        $bundle_fields_option[$parent_field . $value . '_col_' . $column_name] = $level_string  . '- ' . $column_name;
      }
      else {
        if (isset($field_info['settings']['handler_settings']['target_bundles'])) {
          // Build key prefix to make a unique identifier for this particular field.
          $key_prefix = "ref_" . $value . '_';
          $bundle_fields_option['ref_' . $value . '_col_' . $column_name] = $level_string . '- ' . $column_name . ' ' . t("Reference");
          // Add a cosmetic value to make the select item more usable.
          $bundle_fields_option['ref_' . $value . '_col_' . $column_name . '_invalid'] = '--- Referenced Entity Fields ---';
          // Load entity property and fields of referenced bundle. Only support the first bundle for now.
          $ref_properties = entity_get_property_info($field_info['settings']['target_type']);
          $ref_fields = field_info_instances($field_info['settings']['target_type'], current($field_info['settings']['handler_settings']['target_bundles']));
          foreach ($ref_properties['properties'] as $property_name => $property_settings) {
            $bundle_fields_option['ref_' . $value . '_prop_' . $property_name] = $level_string . '-- ' . $property_name;
          }
          // We only load the first level of reference fields for now. Could be changed later to a variable or something else.
          $bundle_fields_option = $bundle_fields_option + _knowledge_graph_build_bundle_field_options($ref_fields, FALSE, ++$level, $value);
          $bundle_fields_option[$key_prefix . '_' . $value . '_col_' . $column_name . '_invalid_end'] = '-------------';
        }
        else {
          $bundle_fields_option[$parent_field . $value . '_col_' . $column_name] = $level_string  . '- ' . $column_name;
        }
      }
    }
  }
  return $bundle_fields_option;
}

/**
 * Dynamically change the fields for the selected bundle.
 */
function _ajax_replace_bundle_fields_callback($form, &$form_state) {
  return $form['fields'];
}

/**
 * Filter bundles for non-mapped bundle types.
 * 
 * @param array $bundles
 *  Array of all bundles existing on page.
 * @param string $type
 *  Type of knowledge graph entity.
 * @return array
 */
function _knowledge_graph_filter_bundles($bundles, $type) {
  $bundles_filtered = array();
  // Get available mappings for filtering.
  $mappings = $result = db_select('knowledge_graph', 'kg')
    ->fields('kg', array('bundle_ref'))
    ->condition('type', $type)
    ->execute()
    ->fetchAll();
  $filter = array();
  foreach ($mappings as $mapping) {
    $filter[] = $mapping->bundle_ref;
  }
  
  // Filter bundles.
  foreach ($bundles as $key => $bundle) {
    if (!in_array($key, $filter)) {
      $bundles_filtered[$key] = $bundle;
    }
  }
  return $bundles_filtered;
}