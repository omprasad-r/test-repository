<?php
/**
 * @file
 * Defines date-related Views data to be able to filter for anniversaries.
 */

/**
 * Implements hook_views_data_alter().
 *
 * Create an additional filter per date field. The new filter allows granular
 * date comparison.
 */
function date_views_custom_filter_granularity_field_views_data_alter(&$result, $field, $module) {
  if ($module == 'date') {
    $has_end_date = !empty($field['settings']['todate']);
    $labels = field_views_field_label($field['field_name']);
    $label = array_shift($labels);

    foreach ($result as $table => $data) {
      $additional = array();
      $field_name = $field['field_name'];
      foreach ($data as $column => $value) {
        // For filters, arguments, and sorts, determine if this column is for
        // the start date ('value') or the end date ('value2').
        $this_column = NULL;
        foreach (array_keys($field['columns']) as $candidate_column) {
          if ($column == $field['field_name'] . '_' . $candidate_column) {
            $this_column = $candidate_column;
            break;
          }
        }

        // Only alter the date fields, not timezone, rrule, offset, etc.
        if ($this_column != 'value' && $this_column != 'value2') {
          continue;
        }

        // Add an additional filter, by copying the original and modifying
        // the filter handler.
        if (array_key_exists('filter', $value)) {
          // New views field column for additional filter.
          $new_column = "{$column}_advanced";

          // Copy the original views field column.
          $result[$table][$new_column] = $result[$table][$column];

          // Remove what is not needed.
          unset($result[$table][$new_column]['argument']);
          unset($result[$table][$new_column]['sort']);

          // Modify filter handler.
          $result[$table][$new_column]['filter']['handler'] = 'date_views_custom_filter_granularity_filter_handler';

          // Replace label.
          if ($has_end_date) {
            switch ($this_column) {
              case 'value':
                // Insert a deliberate double space before 'start date' in the
                // translatable string. This is a hack to get it to appear right
                // before 'end date' in the listing (i.e., in a non-alphabetical,
                // but more user friendly, order).
                $result[$table][$new_column]['title'] = t('@label -  start date (!name) (advanced)', array(
                  '@label' => $label,
                  '!name' => $field['field_name']
                ));
                $result[$table][$new_column]['title short'] = t('@label -  start date (advanced)', array('@label' => $label));
                break;
              case 'value2':
                $result[$table][$new_column]['title'] = t('@label - end date (!name:!column) (advanced)', array(
                  '@label' => $label,
                  '!name' => $field['field_name'],
                  '!column' => $this_column
                ));
                $result[$table][$new_column]['title short'] = t('@label - end date:!column (advanced)', array(
                  '@label' => $label,
                  '!column' => $this_column
                ));
                break;
            }
          }
          else {
            $result[$table][$new_column]['title'] = t('@label (!name) (advanced)', array(
              '@label' => $label,
              '!name' => $field['field_name']
            ));
            $result[$table][$new_column]['title short'] = t('@label (advanced)', array('@label' => $label));
          }
        }
      }
    }
  }
}
