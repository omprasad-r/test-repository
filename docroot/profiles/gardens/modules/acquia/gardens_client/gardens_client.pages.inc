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
      try {
        $verifier = !empty($verifier->module) ? $verifier->module : NULL;
        $message = new \Acquia\Acsf\AcsfMessageRest('POST', 'site-api/v1/verification/resend', array('name' => $name, 'verifier' => $verifier));
        $message->send();
        $response = $message->getResponseBody();
        $sent = $response['sent'];
      }
      catch (Exception $e) {
        watchdog('janrain_client', $e->getMessage(), array(), WATCHDOG_ERROR);
      }

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
