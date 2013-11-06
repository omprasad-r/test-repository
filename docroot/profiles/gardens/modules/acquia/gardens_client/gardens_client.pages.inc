<?php

/**
 * AJAX callback to close the nag box about verifying your email address.
 */
function gardens_client_close_verification_reminder() {
  variable_set('gardens_client_verification_message_muted', REQUEST_TIME);

  $commands = array(  );
  $commands[] = array(
    'command' => 'fadeAlert',
  );

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * AJAX callback to dismiss the most recent twitter alert
 */
function gardens_client_dismiss_alert() {
  global $user;
  $account = user_load($user->uid);

  $alert = variable_get('gardens_alert', array());

  $account->data['alert_dismissed'] = $alert['timestamp'];

  $commands = array(  );
  $commands[] = array(
    'command' => 'fadeAlert',
  );

  variable_set('gardens_alert', $alert);
  user_save($account);

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * AJAX callback to dismiss the aggregator item count warning.
 */
function gardens_client_dismiss_aggregator_alert($level = NULL) {
  $aggregator_warning = variable_get('gardens_limits_aggregator_items_warning', array());
  $warning_timestamp = isset($aggregator_warning['timestamp']) ? $aggregator_warning['timestamp'] : 0;
  $warning_level = isset($aggregator_warning['level']) ? $aggregator_warning['level'] : 0;
  $user_dismissed_data = !empty($user->data['aggregator_alert']) ? $user->data['aggregator_alert'] : array();
  if ($warning_level && (!$user_dismissed_data || $warning_timestamp > $user_dismissed_data['timestamp'] || $warning_level > $user_dismissed_data['level'])) {
    // What we have in $GLOBALS['user'] is not coming from user_load().
    $account = user_load($GLOBALS['user']->uid);
    $account->data['aggregator_alert'] = array(
      'timestamp' => $warning_timestamp,
      'level' => $level && in_array($level, array(80, 90, 100)) ? $level : $warning_level,
    );
    user_save($account);
  }

  $commands = array(
    array('command' => 'fadeAlert'),
  );
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Page callback for gardener OpenID login page.
 */
function gardens_client_gardener_login_page($user_operation = 'login') {
  // Initialized variables for extra OpenID information.
  $is_overlay = FALSE;

  // Allow initiating the login process both in and outside the overlay.
  if (function_exists('overlay_get_mode') && overlay_get_mode() == 'child') {
    if (!user_is_anonymous()) {
      // User left a tab open with login, but is already logged in via another
      // tab. Close the overlay now.
      gardens_client_close_overlay('<front>');
      return '';
    }
    $query = array(
      'render' => 'overlay',
      'destination' => $_GET['destination'] . '?render=overlay',
    );
    $is_overlay = TRUE;
  }
  else {
    $query = array('destination' => $_GET['destination']);
  }

  // Simplified copy of openid_begin() to avoid spending HTTP requests on OpenID
  // discovery. We hardwire the known details of the Gardens provider.
  module_load_include('inc', 'openid');

  // Use identifier_select, so the server tells us the identity, we don't know.
  $claimed_id = $identity = 'http://specs.openid.net/auth/2.0/identifier_select';
  $options = array('absolute' => TRUE, 'query' => $query);
  if (gardens_client_openid_return_secure()) {
    $options['https'] = TRUE;
  }
  $return_to = url('openid/authenticate', $options);
  $service = array(
    'types' => array('http://specs.openid.net/auth/2.0/server', 'http://openid.net/srv/ax/1.0'),
    'uri' => scarecrow_get_gardener_url() . '/openid/provider',
    // Pretend we got anonymous info from the server.
    'identity' => FALSE,
    'version' => 2,
  );

  // Store discovered information in the users' session so we don't have to rediscover.
  $_SESSION['openid']['service'] = $service;
  // Store the claimed id
  $_SESSION['openid']['claimed_id'] = $claimed_id;
  // Store the login form values so we can pass them to
  // user_exteral_login later.
  $_SESSION['openid']['user_login_values'] = array();

  // If a math library is present, then create an association. This will potentially
  // do an HTTP request and store data in the local DB. Used to get data to
  // be able to sign the request for the provider.
  $assoc_handle = '';
  $math_library = _openid_get_math_library();
  if ('gmp' === $math_library || 'bcmath' === $math_library) {
    $assoc_handle = openid_association($service['uri']);
  }

  // Prepare our data to be sent.
  $request = openid_authentication_request($claimed_id, $identity, $return_to, $assoc_handle, $service);
  $request += array(
    'gardens.is_overlay' => $is_overlay,
    'gardens.user_operation' => $user_operation,
  );

  // Output a mock HTML file with a self-submitting form to redirect the user.
  openid_redirect($service['uri'], $request);
}

/**
 * URL redirection callback for gardener user pages.
 *
 * Emails on password request and registration use this to open the respective
 * pages through an overlay on the gardens site.
 */
function gardens_client_gardener_redirection_page() {
  // Get all arguments to pass on to the gardener.
  $args = func_get_args();
  // Redirect to user URL on the gardener with these arguments.
  drupal_goto(scarecrow_get_gardener_url() . '/user/' . join('/', $args), array('query' => array('render' => 'overlay')));
}

/**
 * Check whether openid return callbacks need to be on HTTPS.
 *
 * Currently only checks whether securepages is set to HTTPS for all authenticated
 * traffic.  This covers our current use case, but will need more complex logic if
 * any more complex relationships are needed.
 *
 * @return
 *   TRUE if the callback should use HTTPS, otherwise FALSE.
 */
function gardens_client_openid_return_secure() {
  if (module_exists('securepages')) {
    $roles = variable_get('securepages_roles', array());
    if (is_array($roles) && !empty($roles[DRUPAL_AUTHENTICATED_RID])) {
      // If securepages is enabled and set to SSL for authenticated user role,
      // then we know that the openid callback must be HTTPS, otherwise the user
      // will first land on an HTTP url, and then hop to HTTPS on redirect, which
      // can log the user out. There are most likely improvements in Drupal 7.13,
      // but these don't cover everything.
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * HTTP GET based OpenID login endpoint.
 *
 * Awaits OpenID in GET 'openid' and attempts to start OpenID login with that,
 * finally redirecting the user to the 'node' page (unless a 'destination'
 * query string is already set).
 */
function gardens_login_page() {
  if (user_is_anonymous()) {
    if (!empty($_GET['openid'])) {
      $gardener_host = parse_url(scarecrow_get_gardener_url(), PHP_URL_HOST);
      $openid_host   = parse_url($_GET['openid'], PHP_URL_HOST);
      if ($gardener_host != $openid_host) {
        drupal_set_message(t('Invalid OpenID used for login. Only internal OpenIDs are supported.'), 'error');
      }
      else {
        $options = array(
          'absolute' => TRUE,
          'query' => array('destination' => isset($_REQUEST['destination']) ? $_REQUEST['destination'] : variable_get('site_frontpage', 'node'))
        );
        if (gardens_client_openid_return_secure()) {
          $options['https'] = TRUE;
        }
        $return_to = url('openid/authenticate', $options);
        openid_begin($_GET['openid'], $return_to, array());
      }
      // openid_begin() should have ended with openid_redirect_http() or
      // openid_redirect() if it went well, both of which drupal_exit(), so
      // if we are here, the OpenID login did not go well and the user will
      // get a message about the invalid OpenID. Although they should not even
      // be aware we use OpenID, this should only ever happen if their domain
      // is borked somehow, so we at least expose a tracable error message
      // instead of a whitescreen.
      drupal_goto();
    }
  }
  else {
    // User already logged in, just go to the front page.
    drupal_goto(variable_get('site_frontpage', 'node'));
  }
}

/**
 * AJAX callback to resend a verification email to the site owner.
 *
 * @param $token
 *   A Drupal token from the URL.
 */
function gardens_client_resend_verification_email($token) {
  $commands = array();
  if (drupal_valid_token($token, 'gardens_client_resend_verification_email')) {
    $verifier = new EmailVerifier();
    if ($verifier->isVerified()) {
      $output = t('Your account has recently been verified. Thanks!');
    }
    else {
      // Resend the email.
      $name = $GLOBALS['user']->name;
      $sent = gardens_client_call_gardener('resend.verification.email', array($name, !empty($verifier->module) ? $verifier->module : NULL));
      if (!isset($sent)) {
        $output = '<div class="display-block">' . t('There was an error sending the email. Visit the <a href="@link">site manager</a> to try again.', array('@link' => scarecrow_get_gardener_url() . '/mysites')) . '</div>';
      }
      else {
        $output = t('Email sent.');
      }
    }
  $commands[] = ajax_command_replace('#resend-email-link', $output);
  }

  return array('#type' => 'ajax', '#commands' => $commands);
}
