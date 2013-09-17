<?php
/**
 * @file
 * Date filter with customizable comparison granularity.
 *
 * This filter makes it possible to filter by dates without year
 * restriction, by providing the ability to configure the granularity
 * of the date comparison. Useful in the case of birthdays, national
 * holidays, etc...
 */

class date_views_custom_filter_granularity_filter_handler extends date_views_filter_handler {

  /**
   * Add extra option for setting the date comparison granularity.
   */
  function extra_options_form(&$form, &$form_state) {
    parent::extra_options_form($form, $form_state);

    $form['date_comparison_granularity'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Date comparison granularity'),
      '#description' => t('Specify what date components to use when comparing dates.'),
      '#options' => $this->date_handler->date_parts(),
      '#default_value' => !empty($this->options['date_comparison_granularity']) ? $this->options['date_comparison_granularity'] : array_keys($this->date_handler->date_parts()),
    );
  }

  /**
   * Overrides between operator handling, so that comparison granularity is
   * taken into account.
   */
  function op_between($field) {
    $this->date_combine_conditions('op_between_granular');
  }

  /**
   * Overrides simple operator handling, so that comparison granularity is
   * taken into account.
   */
  function op_simple($field) {
    $this->date_combine_conditions('op_simple_granular');
  }

  /**
   * Overrides parent function, changes comparison operator method calling to
   * use the current level of inheritance.
   */
  protected function date_combine_conditions($function) {
    $this->get_query_fields();
    if (empty($this->query_fields)) {
      return;
    }

    // Create a custom filter group for the conditions.
    $this->query->set_where_group($this->options['date_method'], $this->options['date_group']);
    // Add each condition to the custom filter group.
    foreach ((array) $this->query_fields as $query_field) {
      $field = $query_field['field'];
      $this->date_handler = $query_field['date_handler'];

      // Respect relationships when determining the table alias.
      if ($field['table_name'] != $this->table || !empty($this->relationship)) {
        $this->related_table_alias = $this->query->ensure_table($field['table_name'], $this->relationship);
      }
      $table_alias = !empty($this->related_table_alias) ? $this->related_table_alias : $field['table_name'];
      $field_name = $table_alias . '.' . $field['field_name'];

      // Call the appropriate function, either 'op_between_granular' or 'op_simple_granular'.
      $this->$function($field_name);
    }

    // Gather all of the condition strings and their placeholders.
    $conditions = array();
    $placeholders = array();
    foreach ($this->query->where[$this->options['date_group']]['conditions'] as $condition) {
      $conditions[] = $condition['field'];
      $placeholders += $condition['value'];
    }

    // Remove the conditions from the custom filter group.
    unset($this->query->where[$this->options['date_group']]);

    // Combine all of the conditions into one string.
    $conditions = implode(' ' . $this->options['date_method'] . ' ', $conditions);

    // Add it to the filter group chosen in the Views UI.
    $this->query->add_where_expression($this->options['group'], $conditions, $placeholders);
  }

  /**
   * Between operator, with custom comparison granularity.
   *
   * @see op_between()
   */
  function op_between_granular($field) {

    // Add the delta field to the view so we can later find the value that matched our query.
    list($table_name, $field_name) = explode('.', $field);
    if (!empty($this->options['add_delta']) && (substr($field_name, -6) == '_value' || substr($field_name, -7) == '_value2')) {
      $this->query->add_field($table_name, 'delta');
      $real_field_name = str_replace(array(
        '_value',
        '_value2'
      ), '', $this->real_field);
      $this->query->add_field($table_name, 'entity_id', 'date_id_' . $real_field_name);
      $this->query->add_field($table_name, 'delta', 'date_delta_' . $real_field_name);
    }

    // Obtain date format for comparison.
    $comp_format = date_limit_format($this->format, array_keys(array_filter($this->options['date_comparison_granularity'])));

    $min_value = $this->get_filter_value('min', $this->value['min']);
    $min_comp_date = new DateObject($min_value, date_default_timezone(), $this->format);

    $max_value = $this->get_filter_value('max', $this->value['max']);
    $max_comp_date = new DateObject($max_value, date_default_timezone(), $this->format);

    $field_min = $this->date_handler->sql_field($field, NULL, $min_comp_date);
    // Restrict the date format of the comparison.
    $field_min = $this->date_handler->sql_format($comp_format, $field_min);

    $field_max = $this->date_handler->sql_field($field, NULL, $max_comp_date);
    // Restrict the date format of the comparison.
    $field_max = $this->date_handler->sql_format($comp_format, $field_max);

    // Reformat the min value of the filter.
    $min_value = $min_comp_date->format($comp_format);

    // Reformat the max value of the filter.
    $max_value = $max_comp_date->format($comp_format);

    $placeholder_min = $this->placeholder();
    $placeholder_max = $this->placeholder();

    $group = !empty($this->options['date_group']) ? $this->options['date_group'] : $this->options['group'];

    if ($this->operator == 'between') {
      $this->query->add_where_expression($group, "$field_min >= $placeholder_min AND $field_max <= $placeholder_max", array(
        $placeholder_min => $min_value,
        $placeholder_max => $max_value
      ));
    }
    else {
      $this->query->add_where_expression($group, "$field_min < $placeholder_min OR $field_max > $placeholder_max", array(
        $placeholder_min => $min_value,
        $placeholder_max => $max_value
      ));
    }
  }

  /**
   * Simple operator, with custom comparison granularity.
   *
   * @see op_simple()
   */
  function op_simple_granular($field) {
    // Add the delta field to the view so we can later find the value that matched our query.
    list($table_name, $field_name) = explode('.', $field);
    if (!empty($this->options['add_delta']) && (substr($field_name, -6) == '_value' || substr($field_name, -7) == '_value2')) {
      $this->query->add_field($table_name, 'delta');
      $real_field_name = str_replace(array(
        '_value',
        '_value2'
      ), '', $this->real_field);
      $this->query->add_field($table_name, 'entity_id', 'date_id_' . $real_field_name);
      $this->query->add_field($table_name, 'delta', 'date_delta_' . $real_field_name);
    }

    // Obtain date format for comparison.
    $comp_format = date_limit_format($this->format, array_keys(array_filter($this->options['date_comparison_granularity'])));

    $value = $this->get_filter_value('value', $this->value['value']);
    $comp_date = new DateObject($value, date_default_timezone(), $this->format);

    // Reformat the value of the filter.
    $value = $comp_date->format($comp_format);

    $field = $this->date_handler->sql_field($field, NULL, $comp_date);
    // Restrict the date format of the comparison.
    $field = $this->date_handler->sql_format($comp_format, $field);
    $placeholder = $this->placeholder();
    $group = !empty($this->options['date_group']) ? $this->options['date_group'] : $this->options['group'];
    $this->query->add_where_expression($group, "$field $this->operator $placeholder", array($placeholder => $value));
  }
}
