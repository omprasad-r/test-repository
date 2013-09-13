<?php
/**
 * @file
 * Defines date-related Views data to be able to filter for anniversaries.
 */

/**
 * Implements hook_views_data()
 */
function date_views_custom_filter_granularity_views_data() {
  $data = array();
  // Get date tables.
  $tables = date_views_base_tables();

  foreach ($tables as $base_table => $entity) {
    $data[$base_table]['date_filter_custom_filter_granularity'] = array(
      'group' => t('Date'),
      'title' => t('Custom filter granularity (!base_table)', array('!base_table' => $base_table)),
      'help' => t('Filter any Views !base_table date field by custom granularity.', array('!base_table' => $base_table)),
      'filter' => array(
        'handler' => 'date_views_custom_filter_granularity_filter_handler',
        'empty field name' => t('Undated'),
      ),
    );
  }

  return $data;
}
