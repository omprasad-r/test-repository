<?php
/**
 * @file
 *    Plugin for bunchball groups module. Add user to bunchball group based on profile fields.
 */

class BunchballGroups implements BunchballPluginInterface, BunchballUserInteractionInterface {

  private $options;
  private $nitro;

  function __construct() {
    $this->options = variable_get('bunchball_groups');
    $this->nitro = NitroAPI_Factory::getInstance();
  }

  /**
   * Form callback for this plugin.
   *
   * @param $form
   * @param $form_state
   * @return array
   *    form to be rendered
   */
  public function adminForm($form, &$form_state) {
    $num_rows = 1;
    if (isset($form_state['bunchball_groups_num_rows'])) {
      $num_rows = $form_state['bunchball_groups_num_rows'];
    }
    else {
      if (isset($this->options['groups'])) {
        $num_rows = count($this->options['groups']) + 1;
        $form_state['bunchball_groups_num_rows'] = $num_rows;
      }
    }

    $form['bunchball_groups'] = array(
      '#type' => 'fieldset',
      '#title' => t('Bunchball groups'),
      '#collapsible' => TRUE,
      '#theme' => 'bunchball_groups_admin',
      '#prefix' => '<div id="bunchball-groups-table">',
      '#suffix' => '</div>',
      '#tree' => TRUE,
      '#description' => t('Map users to Bunchball groups according to profile field values.'),
    );
    $form['bunchball_groups']['settings'] = $this->buildFields($num_rows);
    $form['bunchball_groups']['add-button'] = array(
      '#type' => 'submit',
      '#value' => t('Add another row'),
      '#submit' => array('bunchball_groups_ajax_add_one'),
      '#ajax' => array(
        'callback' => 'bunchball_groups_ajax_callback',
        'wrapper' => 'bunchball-groups-table',
      ),
    );

    return $form;
  }

  /**
   * Form validation callback for this plugin.
   *
   * @todo check that checkboxes and action textboxes are consistent
   *
   * @param $form
   * @param $form_state
   */
  public function adminFormValidate($form, &$form_state) {}

  /**
   * Submit callback for this plugin.
   *
   * @param $form
   * @param $form_state
   */
  public function adminFormSubmit($form, &$form_state) {
    $values = isset($form_state['values']['bunchball_groups']['settings']) ?
      $form_state['values']['bunchball_groups']['settings'] : array();
    $this->options['groups'] = array();
    foreach ($values as $key => $value) {
      if (!empty($value['group']['value']) && !empty($value['group']['group']) && empty($value['group']['delete'])) {
        $this->options['groups'][$key] = $value['group'];
      }
    }
    variable_set('bunchball_groups', $this->options);
  }

  /**
   * AJAX callback.
   *
   * @param $form
   * @param $form_state
   * @param $op
   * @param $data
   */
  public function adminFormAjax($form, &$form_state, $op, $data) {
    if ($op == 'addOneRow') {
      $form_state['bunchball_groups_num_rows']++;
      $form_state['rebuild'] = TRUE;
    }
  }

  /**
   * Send action to Bunchball.
   *
   * @param $account
   * @param $op
   */
  public function send($account, $op) {
    if ($op == 'addUserToGroup') {
      try {
        // log in
        foreach ($this->options['groups'] as $group) {
          // field value to check
          $field_name = $group['field'];
          $check_value = $group['value'];
          $add_group = $group['group'];

          $value = field_get_items('user', $account, $field_name);
          $field_info = field_info_field($field_name);
          if (array_key_exists('value', $field_info['columns'])) {
            // we're sure there really is a 'value'
            if ($field_info['cardinality'] == 1) {
              // the field has cardinality 1 -- only case we can handle
              $value_single = (is_array($value) && array_key_exists(0, $value)) ? $value[0]['value'] : NULL;
              if ($value_single == $check_value) {
                $this->nitro->drupalLogin($account);
                $this->nitro->addUserToGroup($add_group);
              }
            }
          }
        }
      }
      catch (NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  /**
   * Build the form fields for a content type.
   *
   * @param $num_rows
   *    Number of rows in the table of rules.
   *
   * @return array
   *    form field elements for one content type
   */
  private function buildFields($num_rows) {
    $form = array();
    $profile_fields = array();
    // get array of profile fields to display as select options
    foreach (field_info_instances('user', 'user') as $name => $details) {
      $profile_fields[$name] = $details['label'];
    }
    if (isset($this->options['groups'])) {
      foreach ($this->options['groups'] as $key => $group) {
        $form[$key] = $this->buildRow($profile_fields, $group);
        $num_rows--;
      }
    }
    for (;$num_rows >0; $num_rows--) {
      $form[] = $this->buildRow($profile_fields);
    }

    return $form;
  }

  /**
   * Create a row of fields.
   *
   * @param $profile_fields
   *    array of custom profile fields
   *
   * @param $group_data
   *    array of stored data for group
   *
   * @return array
   *    row of fields
   */
  private function buildRow($profile_fields, $group_data = array()) {
    $row['group']['field'] = array(
      '#type' => 'select',
      '#options' => $profile_fields,
      '#default_value' => isset($group_data['field']) ? $group_data['field'] : NULL,
    );
    $row['group']['value'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($group_data['value']) ? $group_data['value'] : NULL,
    );
    $row['group']['group'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($group_data['group']) ? $group_data['group'] : NULL,
    );
    if (isset($group_data['field'])) {
      // if not new row
      $row['group']['delete'] = array(
        '#type' => 'checkbox',
        '#default_value' => array(),
      );
    }
    return $row;
  }

}
