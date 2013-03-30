<?php

/**
 * Implements hook_views_data().
 */
function taxonomy_access_views_data() {
  $data = array();
  // Roles with view access to a term
  $data['taxonomy_term_data']['taxonomy_access_view'] = array(
    'title' => t('Roles with view grants'),
    'help' => t('List of roles that have view access to the given term.'),
    'field' => array(
      'handler' => 'taxonomy_access_handler_field_term_grants',
    ),
  );
  return $data;
}
