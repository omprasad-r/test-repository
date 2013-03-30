<?php

/**
 * @file
 * Views integration for Comment on anything.
 */

/**
 * Implements hook_field_views_data().
 */
function comment_on_anything_field_views_data($field) {
  $data = field_views_field_default_views_data($field);

  // Collect information on all entity types that are related to this
  // field. To be used later to set up relationships.
  // @todo: THIS ONLY TAKES THE LAST ONE NOW UNTIL WE FIGURE OUT HOW
  // TO ADD MULTIPLE RELATIONSHIPS TO A VIEWS DATA COLUMN. IF THE FIELD
  // IS APPLIED TO MULTIPLE ENTITY TYPES, THIS WOULD ONLY RELATE IT TO
  // THE LAST ONE FOR NOW.
  foreach ($field['bundles'] as $entity_type => $bundles) {
    $entity_info = entity_get_info($entity_type);
  }

  if (!empty($entity_info) && !empty($entity_type)) {
    foreach ($data as $table_name => $table_data) {
      // Add relationship for background node to list the nodes directly. Users
      // can add an additional comment relationship on top of this to list
      // comments that belong to this node that belongs to the entity.
      // Relationship chain: Entity ===> Content (=> Comment).
      $data[$table_name][$field['field_name'] . '_nid']['relationship'] = array(
        'handler' => 'views_handler_relationship',
        'base' => 'node',
        'base field' => 'nid',
        'group' => t('Comment on anything'),
        'title' => t('Comments node'),
        'label' => t('comment node for !field_name', array('!field_name' => $field['field_name'])),
        'help' => t('Relate the entity to the node that holds the comments.'),
      );

      // Make the above relationship available in the other direction as well.
      // Used when applying the relationship below in practice.
      $data[$table_name]['table']['join']['node'] = array(
        'field' => $field['field_name'] . '_nid',
        'left_field' => 'nid',
      );

      $data[$table_name]['table']['base'] = array(
        'field' => "{$field['field_name']}_nid",
        'field table' => _field_sql_storage_tablename($field),
        'field field' => $field['field_name'],
      );

      // Add relationship for entities related to the nodes being commented on.
      // To be used from comment views that have content relationships which
      // then can have this entity relationship connecting the comment to the
      // entity being commented.
      // Relationship chain: (Comment =>) Content ===> Entity.
      $pseudo_field_name = "reverse_{$field['field_name']}_{$entity_type}";
      $data[$table_name][$pseudo_field_name]['relationship'] = array(
        'handler' => 'views_handler_relationship_entity_reverse',
        'field_name' => $field['field_name'],
        'field table' => _field_sql_storage_tablename($field),
        'field field' => "{$field['field_name']}_nid",
        'base' => $entity_info['base table'],
        'base field' => $entity_info['entity keys']['id'],
        'group' => t('Comment on anything'),
        'title' => t('The entity being commented on'),
        'label' => t('entity for !field_name', array('!field_name' => $field['field_name'])),
        'help' => t('Relate the node that holds the comments to the entity.'),
      );
    }
  }

  return $data;
}
