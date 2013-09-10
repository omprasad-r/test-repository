<?php
/**
 * @file
 * Anniversary date filter.
 * This filter makes the possibility to filter by dates without year restriction
 * (f.e. birthdays, national holidays, etc...)
 */

class date_views_anniversary_filter_handler extends date_views_filter_handler {
  /**
   * Add the selectors to the value form using the date handler.
   */
  function value_form(&$form, &$form_state) {
    // We use different values than the parent form, so we must
    // construct our own form element.
    $form['value'] = array();
    $form['value']['#tree'] = TRUE;
    // Month selector.
    $form['value']['month'] = array(
      '#type' => 'select',
      '#title' => t('Month'),
      '#required' => TRUE,
      '#options' => drupal_map_assoc(range(1,12)),
      '#default_value' => $this->options['value']['month'],
    );
    // Day selector.
    $form['value']['day'] = array(
      '#type' => 'select',
      '#title' => t('Day'),
      '#required' => TRUE,
      '#options' =>  drupal_map_assoc(range(1,31)),
      '#default_value' => $this->options['value']['day'],
    );
    // Add custom css.
    $form['#attached']['css'][] = drupal_get_path('module', 'date_views_anniversary') . '/date_views_anniversary.css';
  }

  /**
   * This filter can't be exposed.
   */
  function can_expose() { return FALSE; }

  /**
   * This filter can't be grouped.
   */
  function can_build_group() { return FALSE;}

  /**
   * Remove operators form, because unnecessary.
   */
  function operator_form() { }

  /**
   * Override options form.
   */
  function extra_options_form(&$form, &$form_state) {
    $fields = date_views_fields($this->base_table);
    $options = array();
    foreach ($fields['name'] as $name => $field) {
      $options[$name] = $field['label'];
    }

    $form['date_fields'] = array(
      '#title' => t('Date field'),
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $this->options['date_fields'],
      '#multiple' => FALSE,
      '#description' => t('Select date field to filter.'),
      '#required' => TRUE,
    );
  }

  // Update the summary values to provide
  // meaningful information for each option.
  function admin_summary() {
    if (empty($this->options['date_fields'])) {
      return t('Missing date fields!');
    }
    $handler = $this->date_handler;

    $fields = date_views_fields($this->view->base_table);
    if (!empty($this->options['date_fields'])) {
      $output = array();
      foreach ($this->options['date_fields'] as $field) {
        if (array_key_exists($field, $fields['name'])) {
          $output[] = $fields['name'][$field]['label'];
        }
      }
    }
    $field = implode(' ' . $this->options['date_method'] . ' ', $output);
    $output = "$field " . check_plain($this->operator) . ' ';
    $output .= $this->get_day() . ' ' . date("F", mktime(0, 0, 0, ($this->get_month())));
    return $output;
  }

  /**
   * Save values.
   */
  function value_submit($form, &$form_state) {
    $this->options['value']['month'] = $form_state['values']['options']['value']['month'];
    $this->options['value']['day'] = $form_state['values']['options']['value']['day'];
  }

  /**
   * Get saved month.
   */
  function get_month() {
    return isset($this->options['value']['month']) ? $this->options['value']['month'] : 0;
  }

  /**
   * Get saved day.
   */
  function get_day() {
    return isset($this->options['value']['day']) ? $this->options['value']['day'] : 0;
  }

  /**
   * Make a query on date fields based on day and month.
   * TODO: Needs to be implemented.
   */
  function query() {

  }
}
