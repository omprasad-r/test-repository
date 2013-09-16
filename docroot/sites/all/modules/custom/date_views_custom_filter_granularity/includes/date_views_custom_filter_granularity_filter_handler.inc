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

class date_views_custom_filter_granularity_filter_handler extends date_views_filter_handler_simple {

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
   * Between operator, with custom comparison granularity.
   *
   * @see op_between()
   */
  function op_between($field) {

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
  function op_simple($field) {
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
