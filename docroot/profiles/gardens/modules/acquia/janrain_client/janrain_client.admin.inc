<?php

/**
 * @file
 *   Admin page callback file for the janrain_client module.
 */

/**
 * Menu callback: Generate a form to configure Engage settings.
 *
 * @ingroup forms
 * @see janrain_client_admin_settings_submit()
 */
function janrain_client_admin_settings($form, &$form_state) {
  $setup = janrain_client_ensure_setup();
  if (!$setup) {
    drupal_set_message(t('There was an error setting up the site for social login and publishing.'), 'error');
    return $form;
  }

  $url = janrain_client_get_rpxnow_url();
  $form['rpxnow_link'] = array(
    '#type' => 'item',
    '#markup' => '<ul class="action-links"><li>' . t('<a href="@url" target="_blank">Add / remove social networks</a>', array('@url' => $url)) . '</li></ul>',
  );

  $providers = janrain_client_get_providers();
  // Save the authorized provider list so that we don't have to call Engage
  // every time we need it (this means site admin will need to revisit this
  // page every time they add/remove a provider in the rpxnow.com dashboard).
  variable_set('rpx_providers', $providers);

  // Add providers. A provider is marked as enabled for social signin if it's
  // on the signin widget. A provider is marked as "can be enabled for social
  // publishing" if it supports social publishing AND it's on the signin widget.
  foreach ($providers as $name => $title) {
    $form['providers'][$name] = array(
      'title' => array(
        '#markup' => theme('rpx_icon', array('provider' => $name, 'size' => 'medium', 'style' => 'rpx-icon-inline')) . '<span' . drupal_attributes(array('class' => 'rpx-provider-title')) . '>' . $title .  '</span>',
      ),
    );
    // If this is a social publishing provider, the admin can enable it for
    // site-wide publishing by connecting her social network account.
    if (janrain_client_is_publishing_provider($name)) {
      $authorized_networks = variable_get('rpx_default_networks');
      if (isset($authorized_networks[$name])) {
        // If the network is enabled, clicking the link will disable it.
        $user_name = $authorized_networks[$name]['name'];
        $query = array('token' => drupal_get_token('janrain_client_disable_default_network'));
        $link = l(t('disable'), "admin/config/people/janrain/disable/$name", array('query' => $query));
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
    }
    else {
      $markup = 'Not supported by provider';
    }
    $form['providers'][$name]['social_publish'] = array (
      '#markup' => $markup,
    );
  }
  $form['#header'] = array(t('Authorized social networks'),t('Social publish'));

  $node_types = node_type_get_types();
  $checkboxes_options = array('comments' => t('Comments'));
  foreach ($node_types as $k => $v) {
    $checkboxes_options[$k] = $v->name;
  }
  $form['publish_node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enable social publishing on:'),
    '#default_value' => variable_get('rpx_publish_node_types', array()),
    '#options' => $checkboxes_options,
  );

  $form['publish_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Publishing templates'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['publish_settings']['content'] = array(
    '#type' => 'fieldset',
    '#title' => t('content'),
    '#collapsible' => FALSE,
  );
  $form['publish_settings']['content']['rpx_default_title_nodes'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => variable_get('rpx_default_title_nodes', RPX_TITLE_STRING_NODES),
    '#description' => t('Default title sent to the user\'s enabled social networks when creating content.'),
  );
  $form['publish_settings']['content']['rpx_default_user_content_nodes'] = array(
    '#type' => 'textfield',
    '#title' => t('Body'),
    '#default_value' => variable_get('rpx_default_user_content_nodes', RPX_USER_CONTENT_STRING_NODES),
    '#description' => t('Default body sent to the user\'s enabled social networks when creating content.'),
  );
  $form['publish_settings']['content']['rpx_default_action_nodes'] = array(
    '#type' => 'textfield',
    '#title' => t('Publish statement'),
    '#default_value' => variable_get('rpx_default_action_nodes', RPX_ACTION_STRING_NODES),
    '#description' => t('Default text sent to the user\'s enabled social networks when creating content.'),
  );

  $form['publish_settings']['comments'] = array(
    '#type' => 'fieldset',
    '#title' => t('comment'),
    '#collapsible' => FALSE,
  );
  $form['publish_settings']['comments']['rpx_default_title_comments'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => variable_get('rpx_default_title_comments', RPX_TITLE_STRING_COMMENTS),
    '#description' => t('Default title sent to the user\'s enabled social networks when commenting.'),
  );
  $form['publish_settings']['comments']['rpx_default_user_content_comments'] = array(
    '#type' => 'textfield',
    '#title' => t('Body'),
    '#default_value' => variable_get('rpx_default_user_content_comments', RPX_USER_CONTENT_STRING_COMMENTS),
    '#description' => t('Default body sent to the user\'s enabled social networks when commenting.'),
  );
  $form['publish_settings']['comments']['rpx_default_action_comments'] = array(
    '#type' => 'textfield',
    '#title' => t('Publish statement'),
    '#default_value' => variable_get('rpx_default_action_comments', RPX_ACTION_STRING_COMMENTS),
    '#description' => t('Default text sent to the user\'s enabled social networks when commenting.'),
  );

  // Add the token tree UI.
  if (module_exists('token')) {
    $form['publish_settings']['token_help'] = array(
      '#theme' => 'token_tree',
      '#token_types' => array('user', 'node', 'comment', 'site'),
      '#global_types' => FALSE,
    );
  }
  else {
    $form['publish_settings']['token_help'] = array(
      '#markup' => t('<a href="@token_module_link" target="_blank">Token</a> module is not enabled. Enable it if you want to use the token browser here.', array('@token_module_link' => 'http://drupal.org/project/token')),
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  // Add the "custom widget" signin JS that will allow admins to "connect"
  // their 3rd party account to the "default networks" list for social
  // publishing.
  janrain_client_signin_to_publish_js(TRUE);
  janrain_client_save_overlay_uri('admin/config/people/janrain');

  // Display message passed by janrain_client_token_handler().
  if (isset($_SESSION['rpx_overlay_message'])) {
    drupal_set_message($_SESSION['rpx_overlay_message']['text'], $_SESSION['rpx_overlay_message']['type']);
    unset($_SESSION['rpx_overlay_message']);
  }

  $form['#tree'] = TRUE;
  return $form;
}

/**
 * Theme the settings form.
 *
 * @ingroup themeable
 */
function theme_janrain_client_admin_settings($variables) {
  $form = $variables['form'];

  $output = drupal_render($form['rpxnow_link']);

  $rows = array();
  foreach (element_children($form['providers']) as $key) {
    $provider = &$form['providers'][$key];

    // Add the row
    $row = array();
    foreach (element_children($provider) as $cell) {
      $row[] = drupal_render($provider[$cell]);
    }
    $rows[] = array('data' => $row);
  }

  $output .= theme('table', array('header' => $form['#header'], 'rows' => $rows));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Submit handler for the main settings form.
 * Save list of publishing providers avaialable to users.
 *
 * @see janrain_client_admin_settings()
 */
function janrain_client_admin_settings_submit($form, &$form_state) {
  $values = $form_state['values'];

  $nodes = $values['publish_node_types'];
  $enabled = array();
  foreach ($nodes as $k => $v) {
    if($v) {
      $enabled[] = $k;
    }
  }
  variable_set('rpx_publish_node_types', $enabled);

  $settings = array_merge($values['publish_settings']['content'], $values['publish_settings']['comments']);
  foreach ($settings as $k => $v) {
    variable_set($k, $v);
  }

  // Signal success message to the user.
  drupal_set_message(t('The configuration options have been saved.'));
}



/**
 * Return a rpxnow.com URL to show in the admin UI.
 *
 * Should be invite URL if the invite is pending, the admin URL otherwise.
 */
function janrain_client_get_rpxnow_url() {
  if (variable_get('rpx_invite_url', '')) {
    // Call the gardener to see if the invite's been accepted since we checked
    // last.
    $result = gardens_client_call_gardener('janrain.check.invite', gardens_client_get_gardens_db_name());
    if ($result) {
      return $result;
    }
    else {
     // Invite's been accepted; no need to check from now on.
      variable_del('rpx_invite_url');
    }
  }

  $url = variable_get('rpx_admin_url', '');
  $url .= $url ? '/providers' : 'https://rpxnow.com';

  return $url;
}

/**
 * Helper function: returns an array of providers sorted according to their
 * weight in the provider weight table.
 */
function _janrain_client_get_sorted_providers($providers) {
  foreach($providers as $k => $v) {
    $providers[$k] = $providers[$k]['weight'];
  }
  asort($providers);
  return array_keys($providers);
}

/**
 * Menu callback; disable (disconnect) a default network (provider) used for
 * social publishing.
 */
function janrain_client_disable_default_network($provider) {
  $networks = variable_get('rpx_default_networks', NULL);
  unset($networks[$provider]);
  variable_set('rpx_default_networks', $networks);
  drupal_goto('admin/config/people/janrain');
}
