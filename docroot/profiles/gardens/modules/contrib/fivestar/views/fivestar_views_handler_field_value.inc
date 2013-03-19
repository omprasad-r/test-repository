<?php

/**
 * @file
 * Provide a views handler for votingapi data fields.
 */

class fivestar_views_handler_field_value extends views_handler_field {
  function construct() {
    parent::construct();
    $this->definition['float'] = TRUE;
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['formatter'] = array(
      'default' => 'fivestar_formatter_default',
    );
    $options['widget']['fivestar_widget'] = array(
      'default' => 'default',
    );
    $options['stars'] = array(
      'default' => 5,
    );
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['formatter'] = array(
      '#type' => 'select',
      '#title' => t('Formatter'),
      '#options' => array(
        'fivestar_formatter_default' => t('As Stars'),
        'fivestar_formatter_rating' => t('Rating (i.e. 4.2/5)'),
        'fivestar_formatter_percentage' => t('Percentage (i.e. 92)'),
      ),
      '#default_value' => $this->options['formatter'],
      '#ajax' => array(
        'path' => views_ui_build_form_url($form_state),
      ),
      '#submit' => array('views_ui_config_item_form_submit_temporary'),
      '#executes_submit_callback' => TRUE,
    );

    if ($this->options['formatter'] == 'fivestar_formatter_default') {
      $form['widget'] = array(
        '#tree' => TRUE,
        '#type' => 'fieldset',
        '#title' => t('Star display options'),
        '#description' => t('Choose a style for your widget.'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $widgets = module_invoke_all('fivestar_widgets');

      $form['widget']['fivestar_widget'] = array(
        '#type' => 'radios',
        '#options' => array('default' => t('Default')) + $widgets,
        '#default_value' => $this->options['widget']['fivestar_widget'],
        '#attributes' => array('class' => array('fivestar-widgets', 'clearfix')),
        '#pre_render' => array('fivestar_previews_expand'),
        '#attached' => array('css' => array(drupal_get_path('module', 'fivestar') . '/css/fivestar-admin.css')),
      );
    }

    $form['stars'] = array(
      '#type' => 'select',
      '#title' => t('Number of stars'),
      '#options' => drupal_map_assoc(range(1, 10)),
      '#default_value' => $this->options['stars'],
    );

  }

  function element_type($none_supported = FALSE, $default_empty = FALSE) {
    return 'div';
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort($order) {
    $this->query->add_orderby(NULL, "COALESCE($this->table_alias.$this->field, 0)", $order, $this->table_alias . '_' . $this->field . '_coalesced');
  }

  function allow_advanced_render() {
    return FALSE;
  }

  function render($values) {
    $rating = $values->{$this->field_alias};
    $stars = $this->options['stars'];

    if ($this->options['formatter'] == 'fivestar_formatter_default') {
      $widgets = module_invoke_all('fivestar_widgets');
      $active = isset($this->options['widget']['fivestar_widget']) ? $this->options['widget']['fivestar_widget'] : 'default';
      $variables = array(
        'rating' => $rating,
        'stars' => $stars,
        'widget' => array(
          'name' => isset($widgets[$active]) ? strtolower($widgets[$active]) : 'default',
          'css' => $active,
        ),
      );
      return theme('fivestar_static', $variables);
    }
    if ($this->options['formatter'] == 'fivestar_formatter_rating') {
      $rating = round(($rating/100) * $stars, 1);
      $output = $rating . '/' . $stars;
      return $output;
    }
    if ($this->options['formatter'] == 'fivestar_formatter_percentage') {
      return round($rating, 1) . '%';
    }
  }
}
