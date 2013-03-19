<?php

/**
 * @file
 * User page callbacks for the janrain_client module.
 */

/**
 * The Engage Sign-in transaction end point for local signins. Engage redirects
 * the user to this URL upon successful authentication by a 3rd party.
 *
 * We also support dialog_user-initiated Engage signins/signups: Engage
 * client-side retrieval is used to post the token to this end point using
 * AJAX.
 *
 * See @link https://rpxnow.com/docs#flow Janrain Engage Flow @endlink
 * See @link http://documentation.janrain.com/social-login-widget/user-s-guide/client-side-retrieval @endlink
 */
function janrain_client_token_handler($signin_to_publish_default) {
  global $user;

  // are we completing an Engage sign-in transaction?
  $token = isset($_REQUEST['token']) ? $_REQUEST['token'] : '';
  // Are we using client-side retrieval?
  $ajax = !empty($_REQUEST['ajax']);

  if (empty($token)) {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    if ($ajax) {
      return drupal_json_output(array('status' => 'error'));
    }
    else {
      drupal_goto();
    }
  }
  $api_key = variable_get('rpx_api_key', '');
  $class = variable_get('janrain_client_class', 'RPX');
  // PHP 5.2 doesn't support direct use of variables for calling
  // static class methods.
  $_SESSION['rpx'] = call_user_func(array($class, 'auth_info'), $token, $api_key, variable_get('rpx_extended_authinfo', FALSE));
  // Clear out any stored error messages and other cruft potentially hanging from previous runs
  foreach (array('rpx_signup_errors', 'rpx_show_validation', 'rpx_no_merge') as $key) {
    if (isset($_SESSION[$key])) {
      unset($_SESSION[$key]);
    }
  }
  // auth_info() stat should be ok.
  if (!$_SESSION['rpx'] || ($_SESSION['rpx']['stat'] != 'ok')) {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    if (isset($_SESSION['rpx']['err']['msg'])) {
      watchdog('janrain_client', 'Failed to obtain a 3rd party identifier for user ID %id: auth_info() returned error: %err', array('%id' => $user->uid, '%err' => $_SESSION['rpx']['err']['msg']), WATCHDOG_ERROR);
    }
    unset($_SESSION['rpx']);
    if ($ajax) {
      return drupal_json_output(array('status' => 'error'));
    }
    else {
      drupal_goto();
    }
  }


  $rpx_id = $_SESSION['rpx']['profile']['identifier'];
  $provider_title = $_SESSION['rpx']['profile']['providerName'];
  $user_name = $_SESSION['rpx']['profile']['preferredUsername'];

  // We are "connecting" a social network for publishing.
  $provider = janrain_client_get_provider_machine_name($provider_title);
  $realm = janrain_client_js_realm();

  if ($signin_to_publish_default) {
    // Some providers (yahoo) don't require configuration for sign-in, but do
    // for publishing. Make sure the provider is configured by checking for
    // the existence of accessCredentials in the auth_info() response.
    if (isset($_SESSION['rpx']['accessCredentials'])) {
      $networks = variable_get('rpx_default_networks', array());
      $networks[$provider] = array(
        'id' => $rpx_id,
        'name' => $user_name,
      );
      variable_set('rpx_default_networks', $networks);
    }
    else {
      watchdog('janrain_client', 'Failed to enable @provider as a default social publish network: make sure it\'s configured in the Janrain Engage <a href="@dashboard">dashboard</a>.', array('@provider' => $provider_title, '@dashboard' => url("https://rpxnow.com/relying_parties/$realm/providers")), WATCHDOG_WARNING);
      $dsm_message = array(
        'text' => t('Failed to enable @provider as a default social publish network: make sure it\'s configured in the Janrain Engage <a href="@dashboard">dashboard</a>.', array('@provider' => $provider_title, '@dashboard' => url("https://rpxnow.com/relying_parties/$realm/providers"))),
        'type' => 'warning',
      );
    }
  }
  else {
    if (isset($_SESSION['rpx']['accessCredentials'])) {
      db_merge('rpx_user_publish_authmap')
        ->key(array(
          'uid' => $user->uid,
          'provider' => $provider,
        ))
        ->fields(array('rpxid' => $rpx_id, 'username' => $user_name))
        ->execute();
    }
    else {
      watchdog('janrain_client', 'Failed to enable @provider as a social publish network: make sure it\'s configured in the Janrain Engage <a href="@dashboard">dashboard</a>.', array('@provider' => $provider_title, '@dashboard' => url("https://rpxnow.com/relying_parties/$realm/providers")), WATCHDOG_WARNING);
      $dsm_message = array(
        'text' => t('Failed to enable @provider as a social publish network.', array('@provider' => $provider_title)),
        'type' => 'warning',
      );
    }
  }
  // Redirect the user back to original overlay if possible.
  if (isset($_SESSION['rpx_overlay_uri'])) {
    $_GET['destination'] = $_SESSION['rpx_overlay_uri'];
    // Store the warning message so that it can be displayed in the overlay
    // instead of its parent window.
    if (isset($dsm_message)) $_SESSION['rpx_overlay_message'] = $dsm_message;
  }
  else {
    unset($_SESSION['rpx_overlay_message']);
    if (isset($dsm_message)) drupal_set_message($dsm_message['text'], $dsm_message['type']);
  }
  drupal_goto();
}

/**
 * Menu callback; manage social networks (publishing) for the specified user.
 */
function janrain_client_user_networks($form, $form_state, $account) {
  drupal_set_title(format_username($account));

  $providers = janrain_client_authorized_publishing_providers();
  $authmap = db_query("SELECT provider, rpxid, username FROM {rpx_user_publish_authmap} WHERE uid = :uid", array(':uid' => $account->uid))->fetchAllAssoc('provider', PDO::FETCH_ASSOC);

  foreach ($providers as $name => $title) {
    $form['providers'][$name] = array(
      'title' => array(
        '#markup' => theme('rpx_icon', array('provider' => $name, 'size' => 'medium', 'style' => 'rpx-icon-inline')) . '<span' . drupal_attributes(array('class' => 'rpx-provider-title')) . '>' . $title .  '</span>',
      ),
    );

    if (isset($authmap[$name])) {
      // If the network is enabled, clicking the link will disable it.
      $user_name = $authmap[$name]['username'];
      $query['token'] = drupal_get_token('janrain_client_disable_user_network');
      $link = l(t('disable'), 'user/' . $account->uid . '/janrain-networks/disable/'. $name, array('query' => $query));
      $markup = "Publish to account:&nbsp;&nbsp;<em>$user_name</em>&nbsp;&nbsp;&nbsp;&nbsp;{$link}";
    }
    else {
      // If the social network is disabled, clicking the link will start the
      // Engage signin process using custom signin UI. This process will
      // result in Engage retrieving from the social network permissions
      // required to post on user's behalf.
      $attributes = array (
        'name' => 'rpx_custom_ui',
        'id' => $name,
        'onclick' => 'return false;',
      );
      $markup = l(t('enable'), '', array('attributes' => $attributes));
    }

    $form['providers'][$name]['social_publish'] = array (
      '#markup' => $markup,
    );
  }

  //
  // Valid node types are the intersection of all node types and those enabled
  // by admin for social sharing.
  //
  $types_all = array('comments' => 'Comments');
  foreach (node_type_get_types() as $k => $v) {
    $types_all[$k] = $v->name;
  }
  $types_admin = array_flip(variable_get('rpx_publish_node_types', array()));
  $types = array_intersect_key($types_all, $types_admin);

  if ($types) {
    // If there are any types for the user to enable/disable, show checkboxes
    // and a save button.
    $types_user = db_select('rpx_user_publish_node_type')
      ->fields('rpx_user_publish_node_type', array('type'))
      ->condition('uid', $account->uid)
      ->execute()
      ->fetchAllAssoc('type', PDO::FETCH_ASSOC);
    $types_user = array_keys($types_user);
    $form['publish_node_types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Enable social publishing on:'),
      '#default_value' => $types_user,
      '#options' => $types,
    );
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  }

  // Add the "custom widget" signin JS that will allow users to "connect"
  // their 3rd party account for social publishing.
  janrain_client_signin_to_publish_js();
  janrain_client_save_overlay_uri('user/' . $account->uid . '/janrain-networks');

  // Display message passed by janrain_client_token_handler().
  if (isset($_SESSION['rpx_overlay_message'])) {
    drupal_set_message($_SESSION['rpx_overlay_message']['text'], $_SESSION['rpx_overlay_message']['type']);
    unset($_SESSION['rpx_overlay_message']);
  }

  $form['#tree'] = TRUE;
  return $form;
}

/**
 * Submit handler for the user social publishing networks form.
 * Save user-selected content types for social publishing.
 *
 * @see janrain_client_user_networks()
 */
function janrain_client_user_networks_submit($form, &$form_state) {
  $uid = $form_state['build_info']['args'][0]->uid;
  $values = $form_state['values'];

  $txn = db_transaction();
  try {
    // Delete old settings for the user.
    db_delete('rpx_user_publish_node_type')
      ->condition('uid', $uid)
      ->execute();

    // Save the new settings.
    $nodes = $values['publish_node_types'];
    foreach ($nodes as $k => $v) {
      if($v) {
        $record = array('uid' => $uid, 'type' => $k);
        drupal_write_record('rpx_user_publish_node_type', $record);
      }
    }
    drupal_set_message(t('Your settings have been saved.'));
  }
  catch (Exception $e) {
    $txn->rollback();
    watchdog_exception('janrain_client', $e);
    drupal_set_message(t('There was a system error and your settings have not been saved.'), 'error');
  }
}

/**
 * Theme the social publishing tab form.
 *
 * @ingroup themeable
 */
function theme_janrain_client_user_networks($variables) {
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form['providers']) as $key) {
    $provider = &$form['providers'][$key];

    // Add the row
    $row = array();
    $row[] = drupal_render($provider['title']);
    $row[] = drupal_render($provider['social_publish']);
    $rows[] = array('data' => $row);
  }
  $header = array(t('Social networks'), t('Social publish'));

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Menu callback; disable (disconnect) a user's network (provider) used for
 * social publishing.
 */
function janrain_client_disable_user_network($account, $provider) {
  db_delete('rpx_user_publish_authmap')
    ->condition('uid', $account->uid)
    ->condition('provider', $provider)
    ->execute();

  drupal_goto('user/' . $account->uid . '/janrain-networks');
}