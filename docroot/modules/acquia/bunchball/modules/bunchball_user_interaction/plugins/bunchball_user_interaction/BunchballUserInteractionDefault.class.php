<?php
/**
 * @file
 * Defines BunchballUserInteractionDefault class
 * that is used as a ctools plugin defined by the bunchball_user_interaction
 * module.
 */

class BunchballUserInteractionDefault implements BunchballUserInteractionInterface, BunchballPluginInterface {

  private $options;

  /**
   * @param $api a class implmenting the NitroAPI interface (could be json or xml)
   */
  function __construct(NitroAPI $api) {
    $actions = array_keys($this->getActions());
    foreach ($actions as $action_name) {
      $this->options[$action_name] = variable_get($action_name, '');
    }
    $this->bunchballApi = $api;
  }

  public function getOptions() {
    return $this->options;
  }

  /**
   * Form callback for plugin.
   */
  public function adminForm($form, &$form_state) {
    $form['bunchball_user_interaction'] = array(
      '#type' => 'fieldset',
      '#title' => t('User Actions'),
      '#collapsible' => TRUE,
      '#description' => t('Enable the user actions to track, which maps them to the Bunchball Nitro console.'),
      '#tree' => TRUE,
    );

    $actions = $this->getActions();
    foreach ($actions as $action_name => $action_details) {
      $form['bunchball_user_interaction'][$action_name . '_check'] = array(
        '#type' => 'checkbox',
        '#title' => $action_details['title'],
        '#description' => $action_details['description'],
        '#default_value' => isset($this->options[$action_name]['enabled']) ? $this->options[$action_name]['enabled'] : array(),
      );

      $form['bunchball_user_interaction'][$action_name . '_action'] = array(
        '#type' => 'textfield',
        '#title' => t('Nitro action name'),
        '#description' => t('The machine name used to map this action to your Bunchball Nitro Server.'),
        '#default_value' => isset($this->options[$action_name]['method']) ? $this->options[$action_name]['method'] : NULL,
        '#states' => array(
          'invisible' => array(
            ':input[name$="' . $action_name . '_check]"]' => array('checked' => FALSE),
          ),
        ),
        '#autocomplete_path' => 'bunchball/actions',
      );
    }

    return $form;
  }

  /**
   * Validation callback for plugin.
   */
  public function adminFormValidate($form, &$form_state) {}

  /**
   * Get availible user actions.
   *
   * @return array
   *   Array of user action names and details translated.
   *   [key] => array(title, description)
   */
  protected function getActions() {
    return array(
      'bunchball_user_login' => array(
        'title' => t('User login'),
        'description' => t('Notify the Bunchball service when a user logs into the site'),
      ),
      'bunchball_user_register' => array(
        'title' => t('User registration'),
        'description' => t('Notify the Bunchball service when a user registers on the site.'),
      ),
      'bunchball_user_profile_complete' => array(
        'title' => t('Profile completion'),
        'description' => t('Notify the Bunchball service with the number of fields a user has completed on their profile.'),
      ),
      'bunchball_user_profile_picture_add' => array(
        'title' => t('Profile picture added'),
        'description' => t('Notify the Bunchball service when a user uploads a profile picture.'),
      ),
      'bunchball_user_profile_picture_update' => array(
        'title' => t('Profile picture updated'),
        'description' => t('Notify the Bunchball service when a user updates a profile picture.'),
      ),
      'bunchball_user_profile_picture_remove' => array(
        'title' => t('Profile picture removal'),
        'description' => t('Notify the Bunchball service when a user removes a profile picture.'),
      ),
    );
  }

  /**
   * Submit callback for plugin.
   */
  public function adminFormSubmit($form, &$form_state) {
    $values = $form_state['values'];

    $bunchball_actions = array_keys($this->getActions());

    foreach ($bunchball_actions as $bunchball_action) {
      if ($values['bunchball_user_interaction'][$bunchball_action . '_check']) {
        $value = array(
          'enabled' => 1,
          'method' => $values['bunchball_user_interaction'][$bunchball_action . '_action'],
        );
        variable_set($bunchball_action, $value);
      }
      else {
        variable_set($bunchball_action, array('enabled' => FALSE, 'method' => ''));
      }
    }
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
   * Callback for user interactions.
   *
   * Send user data to server for specified operation.
   *
   * @param object $account
   *   Drupal user object
   *
   * @param string $op
   *   Operation to send. EG: login, register
   */
  public function send($account, $op) {
    switch ($op) {
      case 'login':
        $this->userLogin($account);
        break;

      case 'register':
        $this->userRegister($account);
        break;

      case 'profileComplete':
        $this->userProfileComplete($account);
        break;

      case 'profilePictureAdd':
        $this->userProfilePictureAdd($account);
        break;

      case 'profilePictureUpdate':
        $this->userProfilePictureUpdate($account);
        break;

      case 'profilePictureRemove':
        $this->userProfilePictureRemove($account);
        break;

    }
  }

  /**
   * A plugin callback that can take a user object and communicate login to
   * bunchball passing whichever arguments the implementer would like.
   *
   * @param object $account
   *   Valid drupal user object
   */
  private function userLogin($account) {
    if (isset($this->options['bunchball_user_login']['enabled']) && $this->options['bunchball_user_login']['enabled']) {
      try {
        $this->apiUserLogin($account);

        $identity_provider = 'Drupal';
        // We call logAction with 'Login' as the 'actionTag'. In addition to the
        // 'actionTag' word we can pass additional tagging information as a comma
        // seperated list of Key/Value pairs (e.g. "Login, Identity Provider: Drupal").
        // If we've got the Janrain module (rpx) we can extract providerName from
        // $account->data and pass that along.

        if (module_exists('rpx_core')){
          if(isset($account->data['rpx_data']['profile']['providerName'])) {
            $identity_provider = $account->data['rpx_data']['profile']['providerName'];
          }
        }
        $action = $this->options['bunchball_user_login']['method'];
        $this->bunchballApi->logAction("$action, Identity Provider: $identity_provider");
      }
      catch (NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
      catch(NitroAPI_HttpException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  /**
   * A plugin callback that can take a user object and communicate regsitration
   * to bunchball.
   *
   * @param object $account
   *   Valid drupal user object
   */
  private function userRegister($account) {
    if (isset($this->options['bunchball_user_register']['enabled']) && $this->options['bunchball_user_register']['enabled']) {
      try {
        $this->apiUserLogin($account);
        $action = $this->options['bunchball_user_register']['method'];
        $this->bunchballApi->logAction($action);
        $this->userProfileComplete($account);
      }
      catch (NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  /**
   * A plugin callback that can take a user object and communicate information
   * about the number of profile fields that have been completed to bunchball.
   *
   * @param object $account
   *   Valid drupal user object
   */
  private function userProfileComplete($account) {
    if ($this->options['bunchball_user_profile_complete']['enabled']) {
      try {
        $count = 0;
        $this->apiUserLogin($account);

        $custom_user_fields = field_info_instances('user', 'user');
        foreach($custom_user_fields as $field => $value) {
          if (isset($account->{$field}[LANGUAGE_NONE][0])) {
            $count++;
          }
        }
        $action = $this->options['bunchball_user_profile_complete']['method'];
        $this->bunchballApi->logAction($action, $count);

      }
      catch (NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  /**
   * A plugin callback that can take a user object and communicate that a
   * profile picture has been uploaded to bunchball passing whichever arguments
   * the implementer would like.
   *
   * @param object $account
   *   A valid drupal user object
   */
  protected function userProfilePictureAdd($account) {
    if ($this->options['bunchball_user_profile_picture_add']['enabled']) {
      try {
        if ($this->isUserPictureUploaded($account) && !$this->isUserPictureAlreadyPresent($account)) {
          $this->apiUserLogin($account);
          $action = $this->options['bunchball_user_profile_picture_add']['method'];
          $this->bunchballApi->logAction($action);
        }
      }
      catch (NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  /**
   * Checks if the user picture is about to be uploaded.
   *
   * @param object $account
   *   Drupal user account.
   *
   * @return Bool
   */
  protected function isUserPictureUploaded($account) {
    return !empty($account->picture_upload);
  }

  /**
   * Checks if the user picture is present on the user object.
   *
   * @param $account object
   *   Drupal user account.
   *
   * @return Bool
   */
  protected function isUserPictureAlreadyPresent($account) {
    return !empty($account->picture);
  }

  /**
   * Checks if the user picture is currently being removed.
   *
   * @param $account object
   *   Drupal user account.
   *
   * @return Bool
   */
  protected function isUserPictureRemoved($account) {
    return !empty($account->picture_delete);
  }

  /**
   * Plugin callback for updating the profile picture.
   *
   * @param $account object
   *   Drupal user account.
   */
  protected function userProfilePictureUpdate($account) {
    if ($this->options['bunchball_user_profile_picture_update']['enabled']) {
      try {
        if ($this->isUserPictureUploaded($account) && $this->isUserPictureAlreadyPresent($account)) {
          $this->apiUserLogin($account);
          $action = $this->options['bunchball_user_profile_picture_update']['method'];
          $this->bunchballApi->logAction($action);
        }
      }
      catch(NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  /**
   * Plugin callback for removing the profile picture.
   *
   * @param $account object
   *   Drupal user account.
   */
  protected function userProfilePictureRemove($account) {
    if ($this->options['bunchball_user_profile_picture_remove']['enabled']) {
      try {
        if ($this->isUserPictureRemoved($account)) {
          $this->apiUserLogin($account);
          $action = $this->options['bunchball_user_profile_picture_remove']['method'];
          $this->bunchballApi->logAction($action);
        }
      }
      catch(NitroAPI_LogActionException $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }

  /**
   * Log in a Drupal user account to bunchball.
   *
   * In this default bunchball.login we are making some assumptions about
   * what bits of information are sent to bunchball to identify a user.
   * @see nitro.api.class::login()
   *
   * @param object $account
   *   Valid drupal user object
   */
  private function apiUserLogin($account) {
    try {
      $this->bunchballApi->drupalLogin($account);
    }
    catch (NitroAPI_LogActionException $e) {
      drupal_set_message($e->getMessage(), 'error');
    }
    catch (NitroAPI_HttpException $e) {
      drupal_set_message($e->getMessage(), 'error');
    }
  }

}
