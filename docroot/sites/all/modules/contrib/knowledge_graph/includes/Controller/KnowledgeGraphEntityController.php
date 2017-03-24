<?php

class KnowledgeGraphEntityController extends EntityAPIControllerExportable {

  public function view($entities, $view_mode = 'full', $langcode = NULL, $page = NULL, $map_content = array()) {

    if (!is_array($map_content)) {
      $map_content = array($map_content);
    }
    switch ($view_mode) {
      case 'full':
        // For Field API and entity_prepare_view, the entities have to be keyed by
        // (numeric) id.
        $entities = entity_key_array_by_property($entities, $this->idKey);
        $output = "";
        if (!empty($this->entityInfo['fieldable'])) {
          field_attach_prepare_view($this->entityType, $entities, $view_mode);
        }
        entity_prepare_view($this->entityType, $entities);
        $langcode = isset($langcode) ? $langcode : $GLOBALS['language_content']->language;

        $view = array();
        foreach ($entities as $entity) {
          $build = entity_build_content($this->entityType, $entity, $view_mode, $langcode);
          $build += array(
            // If the entity type provides an implementation, use this instead the
            // generic one.
            // @see template_preprocess_entity()
            '#theme' => 'entity',
            '#entity_type' => $this->entityType,
            '#entity' => $entity,
            '#view_mode' => $view_mode,
            '#language' => $langcode,
            '#page' => $page,
          );
          // Allow modules to modify the structured entity.
          drupal_alter(array($this->entityType . '_view', 'entity_view'), $build, $this->entityType);
          $key = isset($entity->{$this->idKey}) ? $entity->{$this->idKey} : NULL;
          $view[$this->entityType][$key] = $build;
          $output .= $this->buildJsonLd($entity, $map_content);
        }
        break;

      case 'json-ld':
        $output = "";
        foreach ($entities as $entity) {
          $output .= $this->buildJsonLd($entity, $map_content);
        }

        break;
    }
    return $output;
  }

  /**
   * Main function to build the json-ld output.
   *
   * This function loops over each entity which should get a json-ld output by
   * getting a map which describes the properties, fields and direct fields.
   *
   * @param $knowledge_graph
   *   The map schema for the entities.
   * @param $map_content
   *   An array of entities which get a json-ld output.
   * @return string
   *   An json-ld string.
   */
  public function buildJsonLd($knowledge_graph, $map_content) {
    $knowledge_graph_fields = field_info_instances('knowledge_graph', $knowledge_graph->type);
    $knowledge_graph = entity_metadata_wrapper('knowledge_graph', $knowledge_graph);

    $output = "";
    foreach ($map_content as $entity_id => $entity) {

      // Get the map array to build the json-ld output.
      $entity = entity_metadata_wrapper($knowledge_graph->entity_type_ref->value(), $entity);
      $map = $this->getMap($entity, $knowledge_graph, $knowledge_graph_fields);

      // Build the header of json-ld with required values.
      $json_ld = array(
        '@context' => "http://schema.org",
      );
      $json_ld = $json_ld + $this->buildJson($map);
      $json_ld['location']['@type'] = "Place";
      $json_ld = drupal_json_encode($json_ld);
      $output .= $json_ld;
    }
    return $output;
  }

  /**
   * Returns the map array for a particular entity.
   *
   * The map entity is needed to render the fields from the schema to json.
   *
   * @param $entity
   *  The entity to be mapped.
   * @param $knowledge_graph
   *  The schema of the map.
   * @param $knowledge_graph_fields
   *  The fields of the schema.
   * @return array
   *   entity: The entity which will be mapped.
   *   properties: The properties of the entity
   *   direct_fields: The fields which have been entered directly in the map.
   *   fields: The fields of the entity
   *   ref: Stores a map array of each referenced entity with the same values as in this array.
   */
  public function getMap($entity, $knowledge_graph, $knowledge_graph_fields) {
    $return = array();
    $return = $this->_getEntityMap($entity, $knowledge_graph, $knowledge_graph_fields);
    return $return;
  }

  /**
   * Helper function for getMap().
   * @see $this->getMap().
   */
  private function _getEntityMap($entity, $knowledge_graph,  $fields) {
    $return = array('entity' => $entity, 'properties' => array(), 'direct_fields' => array(), 'fields' => array(), 'ref' => array());

    // Support field groups to allow direct fields to be in a group
    if (module_exists('field_group')) {
      $field_groups = field_group_info_groups("knowledge_graph", $knowledge_graph->type->value());
      $return['field_group'] = isset($field_groups['form']) ? $field_groups['form'] : FALSE;
    }
    foreach ($fields as $field_name => $field) {
      // Fields which are not knowledge_graph_mapper fields are direct fields.
      if ($field['widget']['type'] !== "knowledge_graph_mapper") {
        $value = $knowledge_graph->{$field_name}->value();
        if (empty($value)) {
          continue 1;
        }
        $mapped = FALSE;
        // Check if the field is in a field group, if so, prepend it.
        if (!empty($return['field_group'])) {
          foreach($return['field_group'] as $group_name => $group_settings) {
            if (in_array($field_name, $group_settings->children)) {
              $group = explode("group_", $group_name,2);
              $return['direct_fields'][$group[1]][$field['label']] = $value;
              $mapped = TRUE;
              continue 2;
            }
          }
        }
        if (!$mapped) {
          $return['direct_fields'][$field['label']] = $value;
          continue 1;
        }
      }
      // Knowledge Graph fields will be mapped. Each field has at least
      // a map property and can also belong to a group.
      $map_group = $field['settings']['group'];
      $map_property = $field['settings']['property'];
      // Get stored map value of the field.
      $field_value = $knowledge_graph->{$field['field_name']}->value();
      if (empty($field_value)) {
        continue;
      }
      // Since a field can be referenced field of an entity, we need take
      // care to get the value from the referenced entity.
      if ($this->_is_ref($field_value)) {
        $ref = $this->_get_ref($field_value);
        $field_name = $this->_get_field_name($field_value);
        // If the entity has not been stored, load it and save it.
        if (empty($return['ref'][$field_name])) {
          // Load entity of referenced field.
          $field_setting = field_info_field($field_name);
          $ref_entity =  $entity->{$field_name}->value();
          $ref_entity = entity_metadata_wrapper($field_setting['settings']['target_type'], $ref_entity);
          $return['ref'][$field_name]['entity'] = $ref_entity;
        }
        // Check if the mapped field is a property or a field with a column.
        if ($this->_is_property($ref[1])) {
          $prop = $this->_get_prop($ref[1]);
          $return['ref'][$field_name]['properties'][] =  array('group' => $map_group, 'property' => $map_property, 'map_value' => $prop[1]);
        }
        // If a field is not a reference or a property, we asume it is a normal field.
        else {
          $col = $this->_get_col($ref[1]);
          $ref_field_name_result = $this->_get_ref($field_value);
          $ref_field_name = $ref_field_name_result[1];
          $ref_col_result = $this->_get_col($ref_field_name);
          $ref_col = $ref_col_result[0];
          $ref_field_name_result = "field_" . explode("_field_", $ref_col , 2);
          $ref_field_name = $ref_field_name_result[1];
          $return['ref'][$field_name]['fields'][$ref_field_name][] =  array('group' => $map_group, 'property' => $map_property, 'map_value' =>  $col[1]);
        }
      }
      else {
        // Check if the mapped field is a property or a field with a column.
        if ($this->_is_property($field_value)) {
          $prop = $this->_get_prop($field_value);
          $return['properties'][] = array('group' => $map_group, 'property' => $map_property, 'map_value' => $prop[1]);
        }
        else {
          $col = $this->_get_col($field_value);
          $return['fields'][$col[0]][] =array('group' => $map_group, 'property' => $map_property, 'map_value' => $col[1]);
        }
      }
    }
    return $return;
  }

  /**
   * Use the map and get all values needed to encode in json.
   * @param $map
   * @return array
   *  An array which can be encoded in json.
   */
  public function buildJson($map) {
    // Build json array.
    $json_ld = $map['direct_fields'];
    $properties = $map['properties'];
    foreach ($properties as $property) {
      if (empty($property['group'])) {
        $json_ld[$property['property']] = $map['entity']->$property['map_value']->value();
      }
      else {
        $json_ld[$property['group']][$property['property']] = $map['entity']->$property['map_value']->value();
      }
    }
    $fields = $map['fields'];
    foreach ($fields as $field_name => $field) {
      foreach ($field as $field_variant) {
        $value = $map['entity']->{$field_name}->value();
        // Handle multiple value fields by using the first value. Should be handled differently later.
       $map_value= $this->_get_map_value($map, $field_variant, $field_name, $value);
        if (empty($field_variant['group'])) {
          $json_ld[$field_variant['property']] = $map_value;
        }
        else {
          $json_ld[$field_variant['group']][$field_variant['property']] = $map_value;
        }
      }
   }
    if (!empty($map['ref'])) {
      foreach ($map['ref'] as $ref_map) {
        $json_ld = $json_ld + $this->buildJson($ref_map);
      }
    }
    return $json_ld;
  }

  /**
   * Get a map value of a field in a map.
   * @param $map
   * @param $field_variant
   * @param $field_name
   * @param $value
   * @return string
   */
  private function  _get_map_value($map, $field_variant, $field_name, $value) {
    // Some fields have multiple values, some not. We need to take care of that.
    if (is_array($value)) {
      if (!empty($value[0])) {
        $map_value_result = $map['entity']->{$field_name}->value();
        $map_value = $map_value_result[0][$field_variant['map_value']];
      }
      else {
        $map_value =$value[$field_variant['map_value']];
      }
    }
    else {
      if ($field_variant['map_value'] == 'value') {
        $map_value = $value;
      }
      else {
        $map_value =$value[$field_variant['map_value']];
      }
    }
    // If the field is a date field, we have to take care of the time zone.
    if (!empty($value['date_type']) && $value['date_type'] === 'date') {
      $map_value = format_date(strtotime($map_value . ' ' . $value['timezone_db']), 'custom', 'c', $value['timezone']);
    }

    return $map_value;
  }

  private function _get_field_name($field_value) {
    if ($this->_is_ref($field_value)) {
      $field_value_result = $this->_get_ref($field_value);
      $field_value = $field_value_result[1];
    }
    if ($this->_is_property($field_value)) {
      $property_value_result = $this->_get_prop($field_value);
      return $property_value_result[0];
    }
    else {
      $field_value_result = $this->_get_col($field_value);
      $field_value = explode("_field_", $field_value_result[0] , 2);
      return $field_value[0];
    }
  }

  private function _get_ref($field_value) {
    $field_parts = explode("ref_", $field_value, 2);
    return $field_parts;
  }

  private function _get_prop($field_value) {
    $field_parts = explode("prop_", $field_value, 2);
    return $field_parts;
  }

  private function _get_col($field_value) {
    $field_parts = explode("_col_", $field_value, 2);
    return $field_parts;
  }

  private function _is_ref($field_value) {
    if (strpos($field_value, "ref") !== FALSE ) {
      return TRUE;
    }
    return FALSE;
  }

  private function _is_property($field_value) {
    if (strpos($field_value, "prop") !== FALSE ) {
      return TRUE;
    }
    return FALSE;
  }
}

