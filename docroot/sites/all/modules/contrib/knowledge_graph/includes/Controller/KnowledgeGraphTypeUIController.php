<?php
/**
 * UI controller.
 */
class KnowledgeGraphTypeUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = 'Manage knowledge graph types, including fields.';
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

    // Get node types by name.
    $node_types = node_type_get_names();

    $rows = array();
    foreach ($entities as $entity) {
      // Only show mapping link if any content types are available.
      if (count($node_types)) {
        $additional_col = array(l(t('Add mapping'), "admin/structure/knowledge_graph/map/" . $entity->type));
      } else {
        $additional_col = array(t('No content types available'));
      }
      $rows[] = $this->overviewTableRow($conditions, entity_id($this->entityType, $entity), $entity, $additional_col);
    }
    $additional_header = array('Add new mapping');
    $render = array(
      '#theme' => 'table',
      '#header' => $this->overviewTableHeaders($conditions, $rows, $additional_header),
      '#rows' => $rows,
      '#empty' => t('None.'),
    );
    return $render;
  }
}
/**
 * Generates the knowledge graph type editing form.
 */
function knowledge_graph_type_form($form, &$form_state, $knowledge_graph_type, $op = 'edit', $entity_type = NULL) {

  if ($op == 'clone') {
    // Only label is provided for cloned entities.
    $knowledge_graph_type->label .= ' (cloned)';
    $knowledge_graph_type->type = $entity_type . '_clone';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => isset($knowledge_graph_type->label) ? $knowledge_graph_type->label : '',
  );
  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($knowledge_graph_type->type) ? $knowledge_graph_type->type : '',
    '#machine_name' => array(
      'exists' => 'knowledge_graph_get_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this profile type. It must only contain lowercase letters, numbers, and underscores.'),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save knowledge graph type'),
    '#weight' => 40,
  );
  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function knowledge_graph_type_form_submit(&$form, &$form_state) {
  $knowledge_graph_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  $knowledge_graph_type->save();
  $form_state['redirect'] = 'admin/structure/knowledge_graph/mappings';
}
