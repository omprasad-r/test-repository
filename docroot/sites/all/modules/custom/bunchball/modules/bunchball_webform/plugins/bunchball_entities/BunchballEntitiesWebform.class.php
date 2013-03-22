<?php
/**
 * @file
 *    Ctools plugin for Bunchball Webforms. Send webform submit event to
 *    bunchball.
 */

class BunchballEntitiesWebform implements BunchballPluginInterface, BunchballEntitiesPluginInterface {

  private $options;
  private $nitro;

  function __construct() {
    $this->options = variable_get('bunchball_webform');
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
    $form['bunchball_webform'] = array(
        '#type' => 'fieldset',
        '#title' => t('Webform'),
        '#collapsible' => TRUE,
        '#tree' => TRUE,
    );
    $form['bunchball_webform']['settings'] = $this->buildFields();
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
    $values = $form_state['values']['bunchball_webform']['settings'];
    $this->options['webform']['check'] = $values['webform']['check'];
    $this->options['webform']['action'] = $values['webform']['action'];
    variable_set('bunchball_webform', $this->options);
  }

  /**
   * AJAX callback.
   *
   * @param $form
   * @param $form_state
   * @param $op
   * @param $data
   */
  public function adminFormAjax($form, &$form_state, $op, $data) {}

  /**
   * Register rating actions.
   *
   * @param $id
   * @param $type
   * @param $user
   */
  public function send($id, $type, $user, $op) {
    if ($op == 'webform' && $this->checkSend()) {
      try {
        // log in
        $this->nitro->drupalLogin($user);
        $action = $this->getActionName();
        $this->nitro->logAction($action);
      }
      catch (NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  private function checkSend() {
    return $this->options['webform']['check'];
  }

  private function getActionName() {
    return $this->options['webform']['action'];
  }

  /**
   * Build the form fields for a content type.
   *
   * @return array
   *    form field elements for one content type
   */
  private function buildFields() {
    $form = array();
    $form['webform']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('Submit form'),
      '#description' => t('Notify the Bunchball service when a user submits a webform.'),
      '#default_value' => isset($this->options['webform']['check']) ? $this->options['webform']['check'] : NULL,
    );
    $form['webform']['action'] = array(
      '#type' => 'textfield',
      '#title' => t('Nitro action name'),
      '#description' => t('The machine name used to map this action to your Bunchball Nitro Server.'),
      '#default_value' => isset($this->options['webform']['action']) ? $this->options['webform']['action'] : NULL,
      '#states' => array(
        'invisible' => array(
          ':input[name$="[webform][check]"]' => array('checked' => FALSE),
        ),
      ),
      '#autocomplete_path' => 'bunchball/actions',
    );
    return $form;
  }

}
