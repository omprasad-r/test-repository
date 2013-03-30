<?php
/**
 * @file
 *    Ctools plugin for Bunchball Poll module. Send poll voting events to
 *    bunchball.
 */

class BunchballEntitiesPoll implements BunchballPluginInterface, BunchballEntitiesPluginInterface {

  private $options;
  private $nitro;

  function __construct() {
    $this->options = variable_get('bunchball_poll');
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
    $form['bunchball_poll'] = array(
        '#type' => 'fieldset',
        '#title' => t('Poll votes'),
        '#collapsible' => TRUE,
        '#tree' => TRUE,
    );
    $form['bunchball_poll']['settings'] = $this->buildFields();
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
    $values = $form_state['values']['bunchball_poll']['settings'];
    $this->options['poll']['check'] = $values['poll']['check'];
    $this->options['poll']['action'] = $values['poll']['action'];
    variable_set('bunchball_poll', $this->options);
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
    if ($op == 'vote' && $this->checkSend()) {
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
    return $this->options['poll']['check'];
  }

  private function getActionName() {
    return $this->options['poll']['action'];
  }

  /**
   * Build the form fields for a content type.
   *
   * @return array
   *    form field elements for one content type
   */
  private function buildFields() {
    $form = array();
    $form['poll']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('Vote'),
      '#description' => t('Notify the Bunchball service when a user votes on a poll.'),
      '#default_value' => isset($this->options['poll']['check']) ? $this->options['poll']['check'] : NULL,
    );
    $form['poll']['action'] = array(
      '#type' => 'textfield',
      '#title' => t('Nitro action name'),
      '#description' => t('The machine name used to map this action to your Bunchball Nitro Server.'),
      '#default_value' => isset($this->options['poll']['action']) ? $this->options['poll']['action'] : NULL,
      '#states' => array(
        'invisible' => array(
          ':input[name$="[poll][check]"]' => array('checked' => FALSE),
        ),
      ),
      '#autocomplete_path' => 'bunchball/actions',
    );
    return $form;
  }

}
