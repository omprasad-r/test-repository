<?php

/**
 * @file
 * Page display functions for for Gigya module.
 */

/**
 * Page callback for the user/%user/edit/gigya page.
 *
 * see gigya_menu()
 *
 * @return null|string
 *   Gigya connect page HTML.
 */
function gigya_connect() {
  drupal_set_title(t('Edit your Social Network connections'));
  global $user;
  if ($user->uid === arg(1)) {
    return theme('gigya_connect_display');
  }
  return t('only users can edit thire own connections');
}

/**
 * Socialize-login page callback.
 *
 * see gigya_menu()
 *
 * return array
 *   An array of ajax commands.
 */
function gigya_login_ajax() {
  require_once drupal_get_path('module', 'gigya') . '/GSSDK.php';
  ctools_include('ajax');
  ctools_include('modal');
  $parms = $_POST; // check for XSS line 54.
  /* If the admin set a post login redirect send
  the user to it if no just reload the page. */
  $dest = variable_get('gigya_login_uiconfig_urlredirect', '');
  if (!empty($dest)) {
    $sucess_command = ctools_ajax_command_redirect($dest);
  }
  else {
    $sucess_command = ctools_ajax_command_reload();
  }
  // Check if the user is not logged in.
  if (!user_is_logged_in()) {
    $commands = array();
    $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
    module_load_include('inc', 'gigya');
    $secretkey = variable_get('gigya_SecretKey', '');
    if (SigUtils::validateUserSignature($parms['UID'], $parms['timestamp'], $secretkey, $parms['signature'])) {
      // Check for XSS inside object;
      array_walk_recursive($parms, filter_xss);
      $gigya_user = $parms['user'];

      /* Check to see if the Gigya user is a Drupal user.
      if the uid isn't a Drupal uid then the user_load will return false. */
      if (is_numeric($gigya_user['UID']) && $parms['isSiteUID'] === 'true' && $usr = user_load($gigya_user['UID'])) {
        if (!variable_get('user_email_verification', TRUE)) {
          /* We can suppress the notify login JS because the user is clearly
          already logged into gigya.
          $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
          Log the user in. */
          $form_state =array('uid' => $usr->uid);
          $form = array();
          // A small hack because on variables can be passed by reference
          user_login_submit($form, $form_state);
          $sucess_command[] = $sucess_command;
        }
        else {
          $commands[] = ajax_command_alert(t('You must validate your email address for this account before logging in.'));
        }
        return array('#type' => 'ajax', '#commands' => $commands);
      }
      // If the user isn't a Drupal user, try to register them if allowed.
      elseif (variable_get('user_register', 1)) {
        $form_state['values'] = array();
        $uname = $parms['user']['email'];
        $user_existes = gigya_dose_user_exist($uname);
        if ($user_existes) {
          $form_state = array(
            'title' => t('Please login to link the accounts'),
            'ajax' => TRUE,
            'email' => $parms['user']['email'],
          );
          $commands = ctools_modal_form_wrapper('gigya_link_accounts_form', $form_state);
          return array('#type' => 'ajax', '#commands' => $commands);
        }
        else {
          $form_state['values']['name'] = $uname;
        }
        $form_state['values']['mail'] = $uname;
        $form_state['values']['pass'] = user_password();
        $_SESSION['gigya_uid'] = $parms['user']['UID'];

        // Allow other modules to change this info before we create the user.
        drupal_alter('gigya_create_user', $form_state, $parms);
        drupal_form_submit('user_register_form', $form_state);

        if (form_get_errors()) {
          if (variable_get('gigya_debug', 0)) {
            $messages = drupal_get_messages('error');
            $output = print_r($messages, TRUE);
            // We were unable to register a valid new user.
            // Send the error messages to watchdog.
            watchdog('gigya', 'Unable to auto-create user account:' . $output);
          }
          /* If there is an error in the registration process,
          return the registration form in a modal. */
          $form_state = array(
            'title' => t('Please fill-in missing details'),
            'ajax' => TRUE,
            'gigya' => TRUE,
          );
          $commands = ctools_modal_form_wrapper('user_register_form', $form_state);
          return array('#type' => 'ajax', '#commands' => $commands);
        }
        // The register process is done.
        $commands[] = $sucess_command;
        return array('#type' => 'ajax', '#commands' => $commands);
      }
      else {
        $commands[] = ajax_command_alert(t('Only site administrators can create new user accounts.'));
        return array('#type' => 'ajax', '#commands' => $commands);
      }
    }
    $commands[] = ajax_command_alert(t('Gigya signature not valid.'));
    return array('#type' => 'ajax', '#commands' => $commands);
  }
  $commands[] = $sucess_command;
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * RaaS login callback.
 *
 * @return array
 *   An array or ajax commands
 */
function gigya_raas_login_ajax() {
  if (!empty($_POST)) {
    ctools_include('ajax');
    ctools_include('modal');
    $sucess_command = ctools_ajax_command_reload();
    require_once drupal_get_path('module', 'gigya') . '/GSSDK.php';
    $parms = $_POST; // Post is not displayed on screen no need to filter_xss
    // noting is used except the UID to validate that the call is from Gigya or not
    if (!user_is_logged_in()) {
      $commands = array();
      $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
      module_load_include('inc', 'gigya');
      // Make sure the request came from Gigya.
      $secretkey = variable_get('gigya_SecretKey', '');
      if (SigUtils::validateUserSignature($parms['UID'], $parms['signatureTimestamp'], $secretkey, $parms['UIDSignature'])) {
        // Get the account from Gigya.
        $req_params = array(
          'UID' => $parms['UID'],
          'include' => 'profile,data,emails,loginIDs',
        );
        $gigya_account = _gigya_api("accounts.getAccountInfo", $req_params);
        // Check for XSS inside object;
        array_walk_recursive($gigya_account, filter_xss);
        $email = $gigya_account['profile']['email'];
        $user_name = !empty($gigya_account['loginIDs']['username']) ? $gigya_account['loginIDs']['username'] : $gigya_account['profile']['email'];
        if (empty($email)) {
          $commands[] = ajax_command_alert(t('Email address is required by Drupal and is missing, please contact the site administrator.'));
          return array('#type' => 'ajax', '#commands' => $commands);
        }
        // Check if users exists on the system.
        $user_exist = gigya_dose_user_exist($email);
        if ($user_exist) {
          if (gigya_is_raas_primary_user($gigya_account, $email)) {
            $user = user_load_by_mail($email);
            /* Set global variable so we would know the user as logged in
            RaaS in other functions down the line.*/
            global $_gigya_raas_login;
            $_gigya_raas_login = TRUE;
            $_SESSION['gigya_raas_uid'] = $parms['UID'];
            // A small hack because on variables can be passed by reference
            $form_state =array('uid' => $user->uid);
            $form = array();
            user_login_submit($form, $form_state);
          }
          else {
            /* If this user is not the primary user account in gigya we
            delete the account
            (we don't want two different users with the same email) */
            _gigya_api('accounts.deleteAccount', array('UID' => $parms['UID']));
            // Get info about the primary account from Gigya.
            // @ignore sql_curly
            $query = 'select loginProvider from accounts where loginIDs.emails = ' . $email;
            $search_res = _gigya_api('accounts.search', array('query' => $query));
            $p_provider = $search_res['results'][0]['loginProvider'];
            $sec_provider = $gigya_account['loginProvider'];
            $msg = t('We found your email in our system.<br />Please login to your existing account using your <b>@p_provider</b> identity.<br />
            If you wish to link your account with your <b>@sec_provider</b> identity - after logging-in, please go to your profile page and click the <b>@sec_provider</b> button.',
            array(
              '@p_provider' => $p_provider,
              '@sec_provider' => $sec_provider,
            )
            );
            $commands[] = ctools_modal_command_display('Email all ready exists in system', '<div class="gigya-error">' . $msg . '</div>');
            return array('#type' => 'ajax', '#commands' => $commands);
          }
        }
        // If the user does not exist register him/her.
        else {
          $new_user = array(
            'name' => $user_name,
            'pass' => user_password(),
            'mail' => $email,
            'status' => 1,
            'init' => $email,
          );
          /* custom WMG fields */
          $gigya_birthdate = $gigya_account['profile']['birthYear'] . "-" . sprintf("%02s", $gigya_account['profile']['birthMonth']) . "-" . sprintf("%02s", $gigya_account['profile']['birthDay']);
          $new_user['field_gigya_uid'][LANGUAGE_NONE][0]['value'] = $gigya_account['UID'];
          $new_user['field_first_name'][LANGUAGE_NONE][0]['value'] = $gigya_account['profile']['firstName'];
          $new_user['field_last_name'][LANGUAGE_NONE][0]['value'] = $gigya_account['profile']['lastName'];
          $new_user['field_postal_code'][LANGUAGE_NONE][0]['value'] = $gigya_account['profile']['zip'];
          $new_user['field_birthdate'][LANGUAGE_NONE][0]['value'] = $gigya_birthdate;
          $new_user['field_country'][LANGUAGE_NONE][0]['value'] = $gigya_account['profile']['country'];
          /* Allow other modules to modify the data before user
          is created in drupal database. */
          drupal_alter('gigya_raas_create_user', $new_user, $gigya_account);
          global $user;
          global $_gigya_raas_login;
          $_gigya_raas_login = TRUE;
          $user = user_save(NULL, $new_user);
          $_SESSION['gigya_raas_uid'] = $parms['UID'];
          user_login_finalize();
        }
        if ($user !== FALSE) {
          $commands[] = $sucess_command;
          return array('#type' => 'ajax', '#commands' => $commands);
        }
        else {
          watchdog('gigya-RaaS', 'There was a problem registering a user');
          // TODO: handle this error.
        }
      }
    }
  }
}


/**
 * Get the user friends list from Gigya.
 *
 * @param object $account
 *   The user object.
 *
 * @return string
 *   HTML for friends list.
 */
function gigya_get_friends($account) {
  $title = isset($account->title) ? $account->title : $account->name;
  drupal_set_title(check_plain($title));

  module_load_include('inc', 'gigya');
  $site_friends = array();
  $this_gigya_user = new GigyaUser($account->uid);
  if ($friends = $this_gigya_user->getFriends(array('siteUsersOnly' => TRUE))) {
    if (!empty($friends['friends'])) {
      foreach ($friends['friends'] as $friend) {
        if ($friend['isSiteUser']) {
          $site_friends[] = $friend;
        }
      }
    }
  }
  return theme('gigya_friends', array('friends' => $site_friends));
}

/**
 * Wrapper for the Gigya Javascript userInfo function.
 *
 * @param object $account
 *   The user object.
 *
 * @return string
 *   The user info from Gigya.
 */
function gigya_get_user($account) {
  $title = isset($account->title) ? $account->title : $account->name;
  drupal_set_title(check_plain($title));

  // Add the onload functions.
  $behavior_js = 'Drupal.behaviors.gigyaGetUserInfo = { attach: function(context, settings) { gigya.services.socialize.getUserInfo(Drupal.settings.gigya.conf, {callback:Drupal.gigya.getUserInfoCallback}); }}';
  drupal_add_js($behavior_js, array('type' => 'inline', 'scope' => JS_DEFAULT));

  $userinfo = '<div id="userinfo"></div>';
  return $userinfo;
}


/**
 * AJAX registering callback.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   From state.
 *
 * @return array
 *   An array of ajax commands.
 */
function gigya_register_ajax($form, $form_state) {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_add_js('ajax-responder');
  drupal_process_form('user_register_form', $form, $form_state);
  $commands[] = ctools_modal_command_dismiss();
  $dest = variable_get('gigya_login_uiconfig_urlredirect', '');
  if (!empty($dest)) {
    $commands[] = ctools_ajax_command_redirect($dest);
  }
  else {
    $commands[] = ctools_ajax_command_reload();
  }
  return array('#type' => 'ajax', '#commands' => $commands);
}
