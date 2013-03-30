<?php
/**
 * @file
 *    Ctools plugin for Bunchball user roles.  Set Drupal role for user based
 *    on bunchball level.
 */

class BunchballUserRoles implements BunchballPluginInterface, BunchballUserInteractionInterface {

  private $options;
  private $nitro;

  function __construct() {
    $this->options = variable_get('bunchball_user_roles');
    $this->nitro = NitroAPI_Factory::getInstance();
    $this->nitro->registerCallback($this, 'postLogin', 'postLogin');
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
    $form['bunchball_user_roles'] = array(
      '#type' => 'fieldset',
      '#title' => t('Level synchronization'),
      '#description' => t("Create and approve roles that match your Bunchball levels names. The Bunchball service will apply those roles to a user's account when they are promoted within your Bunchball gamification experience."),
      '#collapsible' => TRUE,
      '#tree' => TRUE,
    );
    $form['bunchball_user_roles']['settings'] = $this->buildFields();

    return $form;
  }

  /**
   * Form validation callback for this plugin.
   *   - not required
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
    $values = $form_state['values']['bunchball_user_roles']['settings'];
    $this->options['roles']['whitelist'] = $values['roles']['whitelist'];
    variable_set('bunchball_user_roles', $this->options);
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
   * Send command to Bunchball.
   *
   * @param $user
   * @param $op
   */
  public function send($user, $op) {
    if ($op == 'setRole') {
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

  private function getActionName() {
    return $this->options['roles']['whitelist'];
  }

  /**
   * Build the form fields for a content type.
   *
   * @return array
   *    form field elements for one content type
   */
  private function buildFields() {
    $form = array();
    $blacklist_roles = array('anonymous user', 'authenticated user', 'administrator');
    $all_roles = user_roles();
    $role_list = drupal_map_assoc(array_diff($all_roles, $blacklist_roles));
    $form['roles']['whitelist'] = array(
      '#type' => 'checkboxes',
      '#title' => t('User roles'),
      '#options' => $role_list,
      '#default_value' => isset($this->options['roles']['whitelist']) ? $this->options['roles']['whitelist'] : NULL,
    );
    return $form;
  }

  /**
   * Update user roles on post-login.
   */
  public function postLogin() {
    $level = $this->nitro->getLevel();
    global $user;
    if (in_array($level, is_array($this->options['roles']['whitelist']) ? $this->options['roles']['whitelist'] : array())) {
      // add role to user
      if ($role = user_role_load_by_name($level)) {
        $user->roles[$role->rid] = $role->name;
        user_save($user);
      }
    }
  }

}
