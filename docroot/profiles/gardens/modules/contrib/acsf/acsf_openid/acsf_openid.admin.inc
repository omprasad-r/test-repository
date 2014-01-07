<?php

/**
 * @file
 * Acquia Site Factory OpenID related admin functions.
 */

/**
 * Form constructor for the user invitation form.
 *
 * @see acsf_openid_admin_invite_people_form_validate()
 * @see acsf_openid_admin_invite_people_form_submit()
 *
 * @ingroup forms
 */
function acsf_openid_admin_invite_people_form($form, &$form_state) {
  $threshold = acsf_openid_admin_invite_people_threshold_get();
  $window = acsf_openid_admin_invite_people_window_get();
  if (empty($_POST) && !flood_is_allowed('acsf_openid_admin_invite_people', $threshold, $window)) {
    drupal_set_message(t('You are only allowed to send @number_per_day invitations per day and have already reached your limit. You will not be able to submit this form until later.', array('@number_per_day' => $threshold)), 'warning');
  }

  $form['email_addresses'] = array(
    '#type' => 'textarea',
    '#title' => t('Enter e-mail addresses for people you would like to invite'),
    '#description' => t('Separate e-mail addresses with commas. Invitations are limited to @threshold a day.', array('@threshold' => $threshold)),
  );
  $form['invitation_message'] = array(
    '#type' => 'textarea',
    '#title' => t('Invitation message'),
    '#description' => t('Available variables are: @tokens.', array('@tokens' => implode(', ', acsf_openid_admin_invite_people_tokens()))),
    '#default_value' => _user_mail_text('register_admin_created_body', NULL, array(), FALSE),
    '#rows' => 9,
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'acsf_openid') . '/acsf_openid.invite.js'),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send invitations'),
  );

  return $form;
}

/**
 * Form validation handler for acsf_openid_admin_invite_people_form().
 */
function acsf_openid_admin_invite_people_form_validate($form, &$form_state) {
  // There must be at least one e-mail address.
  $address_list = trim($form_state['values']['email_addresses']);
  if (empty($address_list)) {
    form_set_error('email_addresses', t('You must enter at least one email address.'));
  }

  // All email addresses must be valid.
  $addresses = array_unique(array_map('trim', explode(',', $address_list)));
  $invalid_addresses = array();
  foreach ($addresses as $address) {
    // We store these email addresses in the database, so make sure they are
    // also within the maximum length.
    if (!valid_email_address($address) || drupal_strlen($address) > EMAIL_MAX_LENGTH) {
      $invalid_addresses[] = $address;
    }
  }
  $number_invalid = count($invalid_addresses);
  if ($number_invalid) {
    $message = format_plural($number_invalid, 'The email address %addresses is invalid.', 'The following email addresses are invalid: %addresses', array('%addresses' => implode(', ', $invalid_addresses)));
    form_set_error('email_addresses', $message);
  }

  // The number of email addresses must not put us over the flood control limit.
  $number_of_addresses = count($addresses);
  $threshold = acsf_openid_admin_invite_people_threshold_get();
  $limit = $threshold - $number_of_addresses + 1;
  $window = acsf_openid_admin_invite_people_window_get();
  // Iterate to find out how many events we have left. (A direct database query
  // would be more efficient, but also more fragile in terms of not using the
  // flood API.)
  while (!flood_is_allowed('acsf_openid_admin_invite_people', $limit, $window) && $number_of_addresses > 0) {
    $number_of_addresses--;
    $limit++;
    $flood_error = TRUE;
  }
  if (!empty($flood_error)) {
    if ($number_of_addresses) {
      form_set_error('email_addresses', format_plural($number_of_addresses, 'You are only allowed to invite 1 user at this time. (You can send a total of @number_per_day invitations per day.)', $number_of_addresses < $threshold ? 'You are only allowed to invite @count users at this time. (You can send a total of @number_per_day invitations per day.)' : 'You are only allowed to invite @count users per day.', array('@number_per_day' => $threshold)));
    }
    else {
      form_set_error('email_addresses', t('You are only allowed to send @number_per_day invitations per day and have already reached your limit. Try again later.', array('@number_per_day' => $threshold)));
    }
  }
}

/**
 * Form submit handler for acsf_openid_admin_invite_people_form().
 */
function acsf_openid_admin_invite_people_form_submit($form, &$form_state) {
  // Set the submitted invitation message to be used from now on.
  variable_set('user_mail_register_admin_created_body', $form_state['values']['invitation_message']);

  $address_list = trim($form_state['values']['email_addresses']);
  $addresses = array_map('trim', explode(',', $address_list));
  $window = acsf_openid_admin_invite_people_window_get();
  $successes = array();
  $failures = array();
  $original_addresses = $addresses;
  $addresses = array_unique($addresses);
  $has_duplicates = count($addresses) != count($original_addresses);
  foreach ($addresses as $address) {
    // Send mail to the address. Pass in an fake 'account' variable so we can
    // use the [user:mail] token, but make sure it has most standard user
    // object properties to avoid PHP notices.
    $account = (object) array_merge((array) drupal_anonymous_user(), db_query("SELECT * FROM {users} WHERE uid = 0")->fetchAssoc());
    $account->mail = $address;
    $message = drupal_mail('user', 'register_admin_created', $address, language_default(), array('account' => $account));
    if ($message['result']) {
      $successes[] = $address;
      // Register the event with the flood control API.
      flood_register_event('acsf_openid_admin_invite_people', $window);
      // Store a record of this invitation.
      acsf_openid_admin_user_invite_save($address);
    }
    else {
      $failures[] = $address;
    }
  }
  if ($number_of_failures = count($failures)) {
    drupal_set_message(format_plural($number_of_failures, 'Mail could not be sent to the following address: %addresses', 'Mail could not be sent to the following addresses: %addresses', array('%addresses' => implode(', ', $failures))), 'error');
  }
  if ($number_of_successes = count($successes)) {
    if ($has_duplicates) {
      drupal_set_message(format_plural($number_of_successes, '1 site registration invitation was sent (duplicate emails were removed). After an invitee registers, they will show up in the list below.', '@count site registration invitations were sent (duplicate emails were removed). After an invitee registers, they will show up in the list below.'));
    }
    else {
      drupal_set_message(format_plural($number_of_successes, '1 site registration invitation was sent. After an invitee registers, they will show up in the list below.', '@count site registration invitations were sent. After an invitee registers, they will show up in the list below.'));
    }
  }
  $form_state['redirect'] = 'admin/people';
}

/**
 * Saves an invited email address to the database.
 *
 * @param $email
 *   The email address to save.
 *
 * @return
 *   TRUE if an invitation was created; FALSE if did not need to be created
 *   because the e-mail address was already invited.
 */
function acsf_openid_admin_user_invite_save($email) {
  $status = db_merge('acsf_openid_invited_users')
    ->key(array('mail' => $email))
    ->execute();

  return $status == MergeQuery::STATUS_INSERT;
}

/**
 * Returns the number of invites that can be sent out in a certain window.
 *
 * @see acsf_openid_admin_invite_people_window_get()
 */
function acsf_openid_admin_invite_people_threshold_get() {
  return variable_get('acsf_openid_invite_people_threshold', 25);
}

/**
 * Returns the window (in seconds) over which site invites are limited.
 *
 * @see acsf_openid_admin_invite_people_threshold_get()
 */
function acsf_openid_admin_invite_people_window_get() {
  // The window is equal to one day. (Note that if we ever change this, there
  // is a lot of text in the form that would need to be changed also.)
  return variable_get('acsf_openid_invite_people_window', 86400);
}

/**
 * Returns an array of tokens for the message inviting users to a site.
 */
function acsf_openid_admin_invite_people_tokens() {
  return array(
    '[site:name]',
    '[site:url]',
    '[site:register-url]',
    '[site:url-brief]',
    '[user:mail]',
  );
}
