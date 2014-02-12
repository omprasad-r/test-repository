<?php

/**
 * @file
 * User page callbacks for the janrain_login module.
 */

/**
 * Menu callback to clear an anonymous user's session data.
 */
function janrain_login_cancel() {
  // Destroy the current session, and reset $user to the anonymous user.
  session_destroy();

  // dialog_user support. @see dialog_user_ajax_callback()
  // @see gardens_features_dialog_user_ajax_menu_callback()
  if (!empty($_GET['ajax'])) {
    $output = array();
    if (!user_is_anonymous() || !($form_info = janrain_login_cancel_get_form_info())) {
      $output[] = dialog_command_reload();
    }
    else {
      dialog_display(TRUE);
      $_SESSION['rpx_signin_dont_hide'] = TRUE;
      $content = drupal_get_form($form_info['id']);
      _janrain_login_rearrange_user_register_login_form($content);
      if (dialog_display()) {
        $content['#action'] = $form_info['path'];
        $output[] = dialog_command_display($content, array('title' => $form_info['title'], 'draggable' => false, 'resizable' => false, ));
      }
      elseif ($destination = $_GET['destination']) {
        $output[] = dialog_command_redirect($destination);
      }
      else{
        $output[] = dialog_command_reload();
      }
    }
    ajax_deliver(array('#type' => 'ajax', '#commands' => $output));
    drupal_exit();
  }
  else {
    drupal_goto();
  }
}

/**
 * The Engage Sign-in transaction end point for local signins. Engage redirects
 * the user to this URL upon successful authentication by a 3rd party.
 *
 * We also support dialog_user-initiated Engage signins/signups: Engage
 * client-side retrieval is used to post the token to this end point using
 * AJAX.
 *
 * See @link http://developers.janrain.com/documentation/engage/ Janrain Engage Flow @endlink
 * See @link http://documentation.janrain.com/social-login-widget/user-s-guide/client-side-retrieval @endlink
 */
function janrain_login_token_handler() {
  global $user;

  // are we completing an Engage sign-in transaction?
  $token = isset($_REQUEST['token']) ? $_REQUEST['token'] : '';
  if (!$token) {
    drupal_set_message(t('You need a token to be here!'), 'error');
    return MENU_ACCESS_DENIED;
  }

  $add_to_account = isset($_REQUEST['add_to_account']) ? TRUE : FALSE;
  // Are we using client-side retrieval?
  $ajax = !empty($_REQUEST['ajax']);

  // We support only local logins to link with remote accounts.
  if (acsf_openid_force_openid()) {
    return MENU_ACCESS_DENIED;
  }
  $api_key = variable_get('rpx_api_key', '');
  $class = variable_get('janrain_client_class', 'RPX');
  // PHP 5.2 doesn't support direct use of variables for calling
  // static class methods.
  $_SESSION['rpx'] = call_user_func(array($class, 'auth_info'), $token, $api_key, variable_get('rpx_extended_authinfo', FALSE));
  // Clear out any stored error messages and other cruft potentially hanging from previous runs
  foreach (array('rpx_signup_errors', 'rpx_show_validation', 'rpx_no_merge') as $key) {
    unset($_SESSION[$key]);
  }
  if (!$_SESSION['rpx']) {
    return janrain_client_token_result($ajax);
  }
  // auth_info() stat should be ok
  if ($_SESSION['rpx']['stat'] != 'ok') {
    watchdog('janrain_login', 'Failed to obtain a 3rd party identifier for user ID %id: auth_info() returned error: %err', array('%id' => $user->uid, '%err' => $_SESSION['rpx']['err']['msg']), WATCHDOG_ERROR);
    return janrain_client_token_result($ajax, 'error', 'We were unable to complete your request.');
  }

  $rpx_id = $_SESSION['rpx']['profile']['identifier'];
  $provider_title = $_SESSION['rpx']['profile']['providerName'];
  $user_name = $_SESSION['rpx']['profile']['preferredUsername'];

  // Save provider info (for token replacement and account linking).
  $_SESSION['rpx_last_provider_info'] = array(
    'title' => $provider_title,
  );

  // Are we adding a new identity to an existing account?
  if ($add_to_account) {
    if (user_is_logged_in()) {
      // the identifier returned by auth_info() should not be already
      // mapped to an existing account
      if (user_get_authmaps($rpx_id)) {
        $message = array(
          'text' => t('We were unable to complete your request.  That account ID is already linked to a user on our site.'),
          'type' => 'error',
        );
      }
      else {
        // Can't use user_set_authmaps() here, since it doesn't support
        // multiple authnames per user via same module
        $aid = db_insert('authmap')
          ->fields(array(
            'uid' => $user->uid,
            'authname' => $rpx_id,
            'module' => 'janrain_client',
          ))
          ->execute();
        db_insert('rpx_authmap_username')
          ->fields(array(
            'aid' => $aid,
            'uid' => $user->uid,
            'username' => $user_name,
          ))
          ->execute();
        // Display no message if account was successfully linked.
        $message = array();
        _janrain_login_import_user_data($user);
        _janrain_login_update_engage_mapping($user->uid);
      }
      unset($_SESSION['rpx']);
      // Redirect the user back to original overlay if possible.
      // @see janrain_client_user_identities().
      if (isset($_SESSION['rpx_overlay_uri'])) {
        $_GET['destination'] = $_SESSION['rpx_overlay_uri'];
        // Store the message for janrain_client_user_identities() so that it
        // can be shown in overlay instead of the parent window.
        if ($message) $_SESSION['rpx_overlay_message'] = $message;
        drupal_goto();
      }
      else {
        unset($_SESSION['rpx_overlay_message']);
        if ($message) drupal_set_message($message['text'], $message['type']);
        drupal_goto('user/' . $user->uid . '/janrain-accounts');
      }
    }
    else {
      unset($_SESSION['rpx']);
      drupal_set_message(t('Please sign in before linking social accounts.'), 'error');
      return MENU_ACCESS_DENIED;
    }
  }

  $account = user_external_load($rpx_id);

  // Is this a registered user?
  if (isset($account->uid)) {
    if (!variable_get('user_email_verification', TRUE) ||
        variable_get('rpx_bypass_email_verification', FALSE) ||
        $account->login ||
        !empty($account->data['rpx_data']['profile']['verifiedEmail']) &&
        strtolower($account->data['rpx_data']['profile']['verifiedEmail']) == strtolower($account->mail)) {
      // IF settings do not require email verification
      // OR
      // it's not the first login for the user (which means the email has
      // already been verified)
      // OR
      // they are using an email the ID provider has already verified
      //
      // then we can skip the email verification process

      // Check that the user has not been blocked.
      $state['values']['name'] = $account->name;
      user_login_name_validate(array(), $state);
      if (!form_get_errors()) {
        // Load global $user and perform final login tasks.
        $form_state['uid'] = $account->uid;
        _janrain_login_import_user_data($account);
        _janrain_login_update_engage_mapping($account->uid);
        user_login_submit(array(), $form_state);
        // Finish Engage signin transaction.
        return janrain_client_token_result($ajax, 'signin');
      }
      // Form errors set the message
      $message = '';
    }
    else {
      $message = t('You must validate your email address for this account before logging in with it.');
    }
    return janrain_client_token_result($ajax, 'signin_failed', $message);
  }
  // New user.
  else {
    // Check that users are allowed to register on their own.
    if (variable_get('user_register', 1)) {
      if (!variable_get('rpx_force_registration_form', FALSE)) {
        $form_state['values'] = array();
        $form_state['values']['op'] = t('Create new account');
        drupal_form_submit('user_register_form', $form_state);
        // See if the user was successfully registered.
        if (!empty($form_state['user'])) {
          // Nothing more to do.
          if ($ajax) {
            return drupal_json_output(array('status' => 'signup'));
          }
          else {
            drupal_goto();
          }
        }
      }
      else {
        drupal_set_message(t('Please complete the registration by filling out the form below.  (If you\'ve previously registered with us, please <a href="@login">log in</a> and add your @provider account under "Linked accounts.")', array('@provider' => $_SESSION['rpx_last_provider_info']['title'], '@login' => url('user/login/nojs'))), 'warning');
      }

      // Redirect to the normal registration page and prefill with the values
      // we received from Engage (see hook_form_alter()).
      // Signal our registration form alter function that automatic user
      // registration failed and we want to keep the captcha (if any) this
      // time around. Also pass the errors so that it sets them on the form
      // before it's shown to the user.
      $_SESSION['rpx_signup_errors'] = form_get_errors();
      // Clear the messages queue. We'll be handling the display of messages
      // ourselves.
      drupal_get_messages('error');

      if ($ajax) {
        return drupal_json_output(array('status' => 'signup_failed'));
      }
      else {
        $destination = drupal_get_destination();
        unset($_GET['destination']);
        drupal_goto('user/register', array('query' => $destination));
      }
    }
    else {
      return janrain_client_token_result($ajax, 'error', 'Only site administrators can create new user accounts.');
    }
  }
}

/**
 * Menu callback; manage Engage 3rd party identities for the specified user.
 *
 * (For local auth only.)
 */
function janrain_login_linked_accounts($account) {
  drupal_set_title(format_username($account));

  $header = array(t('Social networks'), t('Social login'));
  $rows = array();

  $stored_authmap = db_query("SELECT am.aid, authname, username FROM {authmap} am INNER JOIN {rpx_authmap_username} au ON am.aid = au.aid WHERE module = 'janrain_client' AND am.uid = :uid", array(':uid' => $account->uid));
  // authmap does not contain provider names; we need to construct our own that
  // does.
  $authmap = array();
  foreach ($stored_authmap as $identity) {
    $provider = _janrain_client_get_identity_provider($identity->authname);
    $authmap[$provider['name']] = array(
      'authname' => $identity->authname,
      'aid' => $identity->aid,
      'username' => $identity->username,
    );
  }

  $providers = variable_get('rpx_providers', array());

  foreach ($providers as $provider => $provider_title) {
    if (isset($authmap[$provider])) {
      // Clicking the link will delete the linked account.
      $user_name = $authmap[$provider]['username'];
      $query['token'] = drupal_get_token('janrain_login_disable_linked_account');
      $link = l(t('disable'), 'user/' . $account->uid . '/janrain-accounts/disable/'. $authmap[$provider]['aid'], array('query' => $query));
      $markup = "Linked account:&nbsp;&nbsp;<em>$user_name</em>&nbsp;&nbsp;&nbsp;&nbsp;{$link}";
    }
    else {
      // If no account is linked for the provider, clicking the link will start
      // the Engage signin process using custom signin UI. The 3rd party ID will
      // be obtained from the provider and linked to this Drupal user account.
      $attributes = array (
        'name' => 'rpx_custom_ui',
        'id' => $provider,
        'onclick' => 'return false;',
      );
      $markup = l(t('enable'), '', array('attributes' => $attributes));
    }

    $rows[] = array(
      theme('rpx_icon', array('provider' => $provider, 'style' => 'rpx-icon-inline', 'size' => 'medium')) . '<span' . drupal_attributes(array('class' => 'rpx-provider-title')) . '>' . $provider_title .  '</span>',
      $markup
    );
  }

  $build['rpx_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  // Add the "custom widget" signin JS that will allow users to link"
  // their 3rd party accounts for social signin.
  janrain_login_linked_accounts_js();
  janrain_client_save_overlay_uri('user/' . $account->uid . '/janrain-accounts');

  // Display message passed by janrain_client_token_handler().
  if (isset($_SESSION['rpx_overlay_message'])) {
    drupal_set_message($_SESSION['rpx_overlay_message']['text'], $_SESSION['rpx_overlay_message']['type']);
    unset($_SESSION['rpx_overlay_message']);
  }

  return $build;
}

/**
 * Menu callback; Delete the specified Engage identity from the system.
 *
 * (For local auth only.)
 */
function janrain_login_disable_linked_account($account, $aid) {

  $rpx_id = db_query('SELECT authname FROM {authmap} WHERE aid = :aid AND module = :module', array(':aid' => $aid, ':module' => 'janrain_client'))->fetchField();

  if ($rpx_id) {
    db_delete('authmap')
      ->condition('uid', $account->uid)
      ->condition('aid', $aid)
      ->condition('module', 'janrain_client')
      ->execute();

    db_delete('rpx_authmap_username')
      ->condition('aid', $aid)
      ->execute();

    _janrain_login_delete_engage_mapping($rpx_id, $account->uid);
    drupal_goto('user/' . $account->uid . '/janrain-accounts');
  }
  return MENU_NOT_FOUND;
}
