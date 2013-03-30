<?php

/**
 * @file
 * User page callbacks for the janrain_capture module.
 */

/**
 * Callback for the janrain_capture/oauth menu item. This serves as the
 * redirect_uri Capture redirects the user to and performs the authentication.
 */
function janrain_capture_oauth() {
  global $user;
  $token = isset($_REQUEST['code']) ? $_REQUEST['code'] : false;
  // I'm not sure about this origin parameter. It doesn't seem to be used by
  // Drupal or its modules. I think what was meant is 'destination', but let's
  // leave it here for backwards compatibility, in case some site relies on it.
  $origin = isset($_REQUEST['origin']) ? $_REQUEST['origin'] : false;
  if (!$origin) {
    $destination = isset($_REQUEST['destination']) ? $_REQUEST['destination'] : false;
  }
  $url_type = isset($_REQUEST['url_type']) ? $_REQUEST['url_type'] : false;
  $ver = variable_get('janrain_capture_ver', '1.0');

  if ($ver == '1.0') {
    $janrain_capture_fields   = variable_get('janrain_capture_fields', array());
    $janrain_capture_main     = variable_get('janrain_capture_main', array());
    $janrain_capture_optional = variable_get('janrain_capture_optional', array());
  } else {
    $janrain_capture_fields   = variable_get('janrain_capture_fields2', array());
    $janrain_capture_main     = variable_get('janrain_capture_main2', array());
    $janrain_capture_main     = array_merge($janrain_capture_main, variable_get('janrain_capture_ui2', array()));
    $janrain_capture_optional = variable_get('janrain_capture_federate2', array());
    $janrain_capture_optional = array_merge($janrain_capture_optional, variable_get('janrain_capture_backplane2', array()));

    if (isset($_REQUEST['verification_code'])) { $url_type='verify'; }
    // do we need to output like verify or forgot
    if ($url_type) {
      $widget_js = janrain_capture_widget_js();
      $screen = _janrain_capture_get_screen("{$url_type}.html");
      $js = _janrain_capture_get_screen("{$url_type}.js");

      $output = <<<AUTH
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >
  <head>
    <script type="text/javascript">
{$js}
{$widget_js}
    </script>
  </head>
  <body>
{$screen}
  </body>
</html>
AUTH;

      print $output;
      return NULL;
    }
  }

  if ($token) {
    // Cannot use the Drupal url query option because drupal_http_build_query
    // decodes slashes causing incompatibility.
    $redirect_uri = url('janrain_capture/oauth', array('absolute' => TRUE));
    if ($origin) {
      $redirect_uri .= (strpos($redirect_uri, '?') !== FALSE ? '&' : '?') . 'origin=' . urlencode($origin);
    }
    else if ($destination) {
      $redirect_uri .= '?destination=' . str_replace('%2F', '/', rawurlencode($destination));
    }
    $api = new JanrainCaptureApi();

    if ($api->newAccessToken($token, $redirect_uri) == FALSE) {
      $profile = NULL;
    }
    else {
      $profile = $api->loadUserEntity();
    }

    if (!$profile || $profile['stat'] != 'ok') {
      drupal_set_message(t('We were unable to complete your request.'), 'error');
      watchdog('janrain_capture', 'Failed to obtain a Capture record', array(), WATCHDOG_ERROR);
    }
    else {
      // Do we store users' emails in the Drupal database?
      $store_email = !isset($janrain_capture_fields['capture_no_email']) || !$janrain_capture_fields['capture_no_email'];
      if ($store_email) {
        $_SESSION['janrain_capture_email'] = $profile['result']['email'];
      }
      $new_user = FALSE;

      if (!$account = user_external_load($profile['result']['uuid'])) {
        $account = new stdClass();
        $account->name = $profile['result']['email'];
        $account->mail = $store_email ?
          $profile['result']['email'] :
          $profile['result']['uuid'] . '@localhost';
        $account->status = 1;
      }

      if (!isset($account->uid)) {
        // No user was found with our Capture uuid. If we store email addresses in the
        // Drupal database, we can also try matching based on email.
        if ($store_email) {
          // Look for a local user with the same email address.
          $loaded_user = user_load_multiple(array(), array('mail' => $profile['result']['email']));
          $local_user = reset($loaded_user);
        }
        if ($store_email && $local_user) {
          // Are we configured to match users based on email?
          if (isset($janrain_capture_fields['capture_match_email']) && $janrain_capture_fields['capture_match_email']) {
            // Check to see if this user is already mapped to a Capture uuid.
            if (janrain_capture_mapping_exists($local_user->uid)) {
              $mapped_hook = module_invoke_all('janrain_capture_user_already_mapped');
              if (empty($mapped_hook) || !in_array(FALSE, $mapped_hook)) {
                drupal_set_message(t('A user with this email address is already mapped.'), 'error');
              }
            }
            // Bind Capture profile data and uuid to existing user record.
            else {
              user_set_authmaps($local_user, array("authname_janrain_capture" => $profile['result']['uuid']));
              $account = $local_user;
            }
          }
          else {
            $account = FALSE;
            $user_exists_hook = module_invoke_all('janrain_capture_user_exists');
            if (empty($user_exists_hook) || !in_array(FALSE, $user_exists_hook)) {
              drupal_set_message(t('A user with this email address already exists.'), 'error');
            }
          }
        }
        // No local users found. Create a new one.
        else {
          $user_info['pass'] = user_password();
          $account = user_save($account, $user_info);
          $new_user = TRUE;
          if (!$account->uid) {
            $failed_create = module_invoke_all('janrain_capture_failed_create');
            if (empty($failed_create) || !in_array(FALSE, $failed_create)) {
              drupal_set_message(t('Failed to create new user.'), 'error');
            }
          }
          else {
            user_set_authmaps($account, array("authname_janrain_capture" => $profile['result']['uuid']));
          }
        }
      }

      $signin = TRUE;
      if ($account === FALSE) {
        $signin = FALSE;
      }
      if (isset($_SESSION['janrain_capture_action'])
            && ($_SESSION['janrain_capture_action'] == 'finish_third_party'
            || $_SESSION['janrain_capture_action'] == 'legacy_register')
            && isset($janrain_capture_fields['capture_enforce_verification'])
            && $janrain_capture_fields['capture_enforce_verification']
            && $profile['result']['emailVerified'] == NULL) {
        $signin = FALSE;
        if (isset($_SESSION['janrain_capture_email'])){
          drupal_set_message(t('A verification link has been sent to @email. Please check your email.', array('@email' => $_SESSION['janrain_capture_email'])), 'status');
        }
        else {
          drupal_set_message(t('A verification link has been sent. Please check your email.'), 'status');
        }
      }
      elseif (isset($janrain_capture_fields['capture_enforce_verification'])
          && $janrain_capture_fields['capture_enforce_verification']
          && $profile['result']['emailVerified'] == NULL) {
        $signin = FALSE;
        $args = array(
          'action' => 'resend_verification_email',
          'access_token' => $_SESSION['janrain_capture_access_token'],
          'redirect_uri' => url('janrain_capture/resend_verification_email', array('absolute' => TRUE)),
        );
        $resend_link = janrain_capture_url($args);
        $email_unverified = module_invoke_all('janrain_capture_email_unverified', $resend_link);
        if (empty($email_unverified) || !in_array(FALSE, $email_unverified)) {
          drupal_set_message(t('Your email address has not yet been verified. Please check your email and try again. <a href="@resend-link">Click here</a> to have this email resent.', array('@resend-link' => $resend_link)), 'error');
        }
      }

      if ($signin) {
        $form_state['uid'] = $account->uid;
        user_login_submit(array(), $form_state);
        // Re-sync Capture data to make sure we're up to date.
        janrain_capture_sync_account($account, $profile['result']);
        $account = user_save($account);
        module_invoke_all('janrain_capture_user_authenticated', $profile['result'], $account, $new_user);
      }
    }
  }
  else {
    $no_oauth = module_invoke_all('janrain_capture_no_oauth');
    if (empty($no_oauth) || !in_array(FALSE, $no_oauth)) {
      drupal_set_message(t('No Oauth token found!'), 'error');
    }
  }
  $front_page = url('<front>', array('absolute' => TRUE));
  $signin_redirect = url('janrain_capture/signin_redirect', array('absolute' => TRUE));

  $output = <<<OAUTH_END
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >
  <body>
    <p>Please wait...</p>
    <script type="text/javascript">
      var redirect_url = null;
      var regex = /[\#\?]destination\=([^\&]*)/;
      var match = regex.exec(window.parent.location.href);
      if (match && match.length == 2) {
        redirect_url = "{$signin_redirect}/?destination=" + decodeURIComponent(match[1]);
      }
      if (window.location.href != window.parent.location.href) {
        if (window.parent.location.href.indexOf("logout") > 1) {
          window.parent.location.href = "{$front_page}";
        } else {
          if (redirect_url) {
            window.parent.location.href = redirect_url;
          } else {
            window.parent.location.reload();
          }
        }
      } else {
        window.location.href = redirect_url || "{$front_page}";
      }
    </script>
  </body>
</html>
OAUTH_END;

  print $output;
  return NULL;
}

/**
 * Callback to redirect the user to the destination they were looking for before
 * signing in via Capture.
 */
function janrain_capture_signin_redirect() {
  drupal_goto();
}

/**
 * Callback for the janrain_capture/profile menu item. Ensures the access_token
 * is valid before redirecting the user to the Capture profile screen.
 */
function janrain_capture_profile() {
  $method = isset($_GET['method']) ? $_GET['method'] : '';
  $callback = isset($_GET['callback']) ? $_GET['callback'] : 'Drupal.janrainCapture.closeProfileEditor';
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());
  $redirect_uri = url('janrain_capture/oauth', array('absolute' => TRUE));

  if (REQUEST_TIME >= $_SESSION['janrain_capture_expires_in']) {
    $api = new JanrainCaptureApi();
    $api->refreshAccessToken();
  }

  $args = array(
    'action' => 'profile' . $method,
    'access_token' => $_SESSION['janrain_capture_access_token'],
    'callback' => $callback,
  );

  $url = janrain_capture_url($args);

  drupal_goto($url);
}

/**
 * Callback for the janrain_capture/profile_sync menu item. Retrieves
 * the most recent data from Capture and stores values locally.
 */
function janrain_capture_profile_sync() {
  global $user;

  $api = new JanrainCaptureApi();
  $profile = $api->loadUserEntity();

  if (!$profile) {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to obtain a Capture record', array(), WATCHDOG_ERROR);
    return;
  }

  // Make sure the account object is fully populated.
  $account = user_load($user->uid);
  janrain_capture_sync_account($account, $profile['result']);
  if ($account = user_save($account)) {
    $profile_updated_hook = module_invoke_all('janrain_capture_user_profile_updated', $profile['result'], $account);
    if (empty($profile_updated_hook) || !in_array(FALSE, $profile_updated_hook)) {
      drupal_goto();
    }
  }
  else {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to save Capture data to user', array(), WATCHDOG_ERROR);
  }
}

/**
 * Callback for the janrain_capture/resend_verification_email menu item.
 * Displays a confirmation message that a verification email was resent.
 */
function janrain_capture_resend_verification_email() {
  $hook = module_invoke_all('janrain_capture_verification_resent');
  if (empty($hook) || !in_array(FALSE, $hook)) {
    if (isset($_SESSION['janrain_capture_email'])) {
      drupal_set_message(t('A verification link has been sent to @email. Please check your email.', array('@email' => $_SESSION['janrain_capture_email'])), 'status');
    }
    else {
      drupal_set_message(t('A verification link has been sent. Please check your email.'), 'status');
    }
    drupal_goto();
  }
}

/**
 * Callback the janrain_capture/token_expired menu item. Logs out a user
 * due to an expired session.
 */
function janrain_capture_token_expired() {
  global $user;
  $tmp = NULL;
  session_destroy();
  user_module_invoke('logout', $tmp, $user);
  $user = drupal_anonymous_user();

  drupal_set_message(t('Your session has expired. Please sign in again.'), 'error');
  drupal_goto();
}
